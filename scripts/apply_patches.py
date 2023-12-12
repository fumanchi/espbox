from os import path
from os.path import join, isfile

Import("env")


pathPatches = path.join(env["PROJECT_DIR"], "patches")
pathPatchFile = path.join(pathPatches, "ESP_WiFiManager-Impl.patch")

pathESPWifiManagerFolder = path.join(env["PROJECT_LIBDEPS_DIR"], env["PIOENV"], "ESP_WifiManager")
pathESPWifiManagerPatchflagFile = path.join(pathESPWifiManagerFolder, ".patching-done")
pathESPWifiManagerSrcFolder = path.join(pathESPWifiManagerFolder, "src")
pathpathESPWifiManagerSrcFile = path.join(pathESPWifiManagerSrcFolder, "ESP_WiFiManager-Impl.h")


# patch file only if we didn't do it before
if not isfile(pathESPWifiManagerPatchflagFile):
    assert isfile(pathpathESPWifiManagerSrcFile)
    assert isfile(pathPatchFile)

    env.Execute("patch %s %s" % (pathpathESPWifiManagerSrcFile, pathPatchFile))

    def _touch(path):
        with open(path, "w") as fp:
            fp.write("")

    env.Execute(lambda *args, **kwargs: _touch(pathESPWifiManagerPatchflagFile))
