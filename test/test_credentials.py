"""Offline checks: python3 -m unittest discover -s test -p 'test_*.py'."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]

class WeatherCredentialTests(unittest.TestCase):
    def check_header(self, key=None):
        compiler = shutil.which(os.getenv("CXX", "c++"))
        self.assertIsNotNone(compiler, "Install a host C++ compiler")
        with tempfile.TemporaryDirectory() as work:
            work = Path(work)
            shutil.copyfile(ROOT / "include/weather_credentials.h", work / "weather_credentials.h")
            if key is not None:
                (work / "duduclock_secrets.h").write_text(
                    f'#define DUDUCLOCK_QWEATHER_API_KEY "{key}"\n', encoding="utf-8"
                )
            (work / "test.cpp").write_text(
                '#include <cstring>\n#include "weather_credentials.h"\n'
                'int main(int argc, char** argv) {\n'
                '  if (argc != 2) return 2;\n'
                '  return weatherConfigured() != (argv[1][0] != 0) ||\n'
                '    std::strcmp(weatherApiKey, argv[1]) != 0;\n'
                '}\n', encoding="utf-8"
            )
            subprocess.run([compiler, "-std=gnu++11", "-Wall", "-Wextra", "-Werror",
                            "test.cpp", "-o", "test"], cwd=work, check=True)
            subprocess.run([str(work / "test"), key or ""], check=True)

    def test_missing_local_config(self):
        self.check_header()

    def test_blank_local_config(self):
        self.check_header("")

    def test_dummy_local_config(self):
        self.check_header("offline_fixture_only")

    def test_no_secrets_in_sensitive_output_or_defaults(self):
        common = (ROOT / "src/common.h").read_text(encoding="utf-8")
        net = (ROOT / "src/net.cpp").read_text(encoding="utf-8")
        prefs = (ROOT / "src/preferencesUtil.cpp").read_text(encoding="utf-8")
        self.assertNotRegex(common, r'[0-9a-fA-F]{32}')
        self.assertNotIn("api.ipshudi.com", common)
        self.assertNotIn("connectDefaultWiFi", net)
        self.assertNotIn("setInfo4Test", prefs)
        for line in net.splitlines():
            if "log_" in line or "Serial." in line or "server.send(" in line:
                self.assertNotRegex(line, r'\b(?:url|payload|pass|weatherApiKey)\b|\bdata\.c_str\(')

    def test_weather_errors_do_not_reset_clock_configuration(self):
        net = (ROOT / "src/net.cpp").read_text(encoding="utf-8")
        lookup = net.split("int getCityID(){", 1)[1].split("void getNowWeather(){", 1)[0]
        self.assertNotIn("clearWiFiCity", lookup)
        self.assertNotIn("restartSystem", lookup)
        for name in ("getNowWeather", "getAir", "getFutureWeather"):
            start = net.split(f"void {name}(){{", 1)[1].split("String url", 1)[0]
            self.assertIn("if (!weatherConfigured() || location.isEmpty()) return;", start)

if __name__ == "__main__":
    unittest.main()
