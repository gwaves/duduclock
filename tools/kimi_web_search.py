#!/usr/bin/env python3
"""
Kimi Web Search CLI - 调用 Kimi API 的联网搜索功能

用法:
    export MOONSHOT_API_KEY="your-api-key"
    python3 kimi_web_search.py "查询内容"
    python3 kimi_web_search.py --model kimi-k2.5 "查询内容"
    python3 kimi_web_search.py --raw "查询内容"   # 输出原始 JSON

注意:
    - 使用内置工具 $web_search，必须禁用 thinking
    - 需要两轮 API 调用：触发搜索 → 回填结果
"""

import argparse
import json
import os
import sys
import urllib.request

API_BASE = "https://api.moonshot.cn/v1"


def api_call(endpoint: str, payload: dict, api_key: str) -> dict:
    url = f"{API_BASE}/{endpoint}"
    data = json.dumps(payload, ensure_ascii=False).encode("utf-8")
    req = urllib.request.Request(
        url,
        data=data,
        headers={
            "Content-Type": "application/json",
            "Authorization": f"Bearer {api_key}",
        },
        method="POST",
    )
    with urllib.request.urlopen(req, timeout=120) as resp:
        return json.loads(resp.read().decode("utf-8"))


def search(query: str, api_key: str, model: str = "kimi-k2.6") -> dict:
    system_msg = "你是Kimi，由Moonshot AI提供的人工智能助手。"

    # 第一轮：触发搜索
    round1 = api_call(
        "chat/completions",
        {
            "model": model,
            "messages": [
                {"role": "system", "content": system_msg},
                {"role": "user", "content": query},
            ],
            "tools": [
                {
                    "type": "builtin_function",
                    "function": {"name": "$web_search"},
                }
            ],
            "thinking": {"type": "disabled"},
        },
        api_key,
    )

    choice = round1["choices"][0]
    message = choice["message"]

    # 如果模型没有触发搜索，直接返回
    if choice.get("finish_reason") != "tool_calls" or "tool_calls" not in message:
        return round1

    tool_calls = message["tool_calls"]

    # 构建第二轮消息
    messages = [
        {"role": "system", "content": system_msg},
        {"role": "user", "content": query},
        {
            "role": "assistant",
            "tool_calls": tool_calls,
        },
    ]
    for tc in tool_calls:
        messages.append({
            "role": "tool",
            "tool_call_id": tc["id"],
            "name": tc["function"]["name"],
            "content": tc["function"]["arguments"],
        })

    # 第二轮：回填结果，获取最终回答
    round2 = api_call(
        "chat/completions",
        {
            "model": model,
            "messages": messages,
            "tools": [
                {
                    "type": "builtin_function",
                    "function": {"name": "$web_search"},
                }
            ],
            "thinking": {"type": "disabled"},
        },
        api_key,
    )

    return round2


def main():
    parser = argparse.ArgumentParser(description="Kimi Web Search CLI")
    parser.add_argument("query", help="搜索查询内容")
    parser.add_argument("--model", default="kimi-k2.6", help="模型名称 (默认: kimi-k2.6)")
    parser.add_argument("--raw", action="store_true", help="输出原始 JSON 响应")
    args = parser.parse_args()

    api_key = os.environ.get("MOONSHOT_API_KEY")
    if not api_key:
        print("错误: 请设置环境变量 MOONSHOT_API_KEY", file=sys.stderr)
        sys.exit(1)

    try:
        result = search(args.query, api_key, args.model)
    except urllib.error.HTTPError as e:
        print(f"HTTP 错误 {e.code}: {e.read().decode()}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"请求失败: {e}", file=sys.stderr)
        sys.exit(1)

    if args.raw:
        print(json.dumps(result, ensure_ascii=False, indent=2))
        return

    choice = result["choices"][0]
    msg = choice["message"]
    content = msg.get("content", "")

    if content:
        print(content)
    else:
        print(json.dumps(result, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
