using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Web.Script.Serialization;
using System.Windows.Forms;

internal static class PortableLauncher
{
    private static readonly string Root = AppDomain.CurrentDomain.BaseDirectory.TrimEnd(Path.DirectorySeparatorChar);
    private static readonly JavaScriptSerializer Json = new JavaScriptSerializer { MaxJsonLength = 128 * 1024 * 1024, RecursionLimit = 200 };
    private static readonly UTF8Encoding Utf8 = new UTF8Encoding(false);
    private static readonly string[] ResourceFolders = { "Fonts", "Character", "Deploy", "Effect", "Map", "Sound", "UI" };

    private static string BundlePath(string relative)
    {
        if (String.IsNullOrWhiteSpace(relative) || Path.IsPathRooted(relative) || relative.Contains(":") ||
            relative.Split('/', '\\').Any(part => part == ".." || part == "." || part.Length == 0))
            throw new IOException("Unsafe package path: " + relative);
        string full = Path.GetFullPath(Path.Combine(Root, relative.Replace('/', Path.DirectorySeparatorChar)));
        if (!full.StartsWith(Root + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase))
            throw new IOException("Package path escaped the extracted folder.");
        string cursor = full;
        while (cursor.Length > Root.Length)
        {
            if ((File.Exists(cursor) || Directory.Exists(cursor)) && (File.GetAttributes(cursor) & FileAttributes.ReparsePoint) != 0)
                throw new IOException("Package paths must not redirect: " + relative);
            cursor = Path.GetDirectoryName(cursor);
        }
        return full;
    }

    private static Dictionary<string, object> ReadObject(string relative)
    {
        string path = BundlePath(relative);
        if (new FileInfo(path).Length > 128L * 1024 * 1024) throw new IOException("Package JSON is too large: " + relative);
        return Json.Deserialize<Dictionary<string, object>>(File.ReadAllText(path, Encoding.UTF8));
    }

    private static string Hash(string path)
    {
        using (var algorithm = SHA256.Create())
        using (var stream = File.OpenRead(path))
            return BitConverter.ToString(algorithm.ComputeHash(stream)).Replace("-", "").ToLowerInvariant();
    }

    private static ulong Revision(string path, string key) { return Convert.ToUInt64(ReadObject(path)[key]); }

    private const string SaveReceipt = "Server/Bin/DataFiles/Gameplay/BalanceNumeric.save.receipt.json";
    private const string ActiveReceipt = "Server/Bin/DataFiles/Gameplay/NumericBalance.active.json";
    private const string Bootstrap = "Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap";
    private static readonly HashSet<string> MutableNumericFiles = new HashSet<string>(StringComparer.Ordinal) {
        "Data/Balance/PlayerProfiles.json", "Data/Balance/PlayerSkills.json", "Data/Balance/DamageProfiles.json",
        "Data/Balance/BossProfiles.json", "Data/Balance/Profiles/Retail.balanceprofile.json",
        "Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json",
        "Data/Valtan/Valtan.legacy-compatibility.json", "Data/Encounters/Valtan/ValtanEncounter.json",
        "Data/Valtan/Valtan.gameplay.json",
        Bootstrap, ActiveReceipt,
        "Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json", "Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json",
        "Data/Balance/NumericSourceBindings.json",
        "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json",
        "Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json"
    };
    private static bool IsHash(string value) {
        return value != null && value.Length == 64 && value.All(c => (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    }
    private static void ExactKeys(Dictionary<string, object> value, params string[] keys) {
        if (value == null || value.Count != keys.Length || keys.Any(key => !value.ContainsKey(key)))
            throw new IOException("Malformed numeric save receipt.");
    }
    private static Dictionary<string, string> NumericOverrides()
    {
        var hashes = new Dictionary<string, string>(StringComparer.Ordinal);
        if (!File.Exists(BundlePath(SaveReceipt))) return hashes;
        var saved = ReadObject(SaveReceipt);
        ExactKeys(saved, "schema", "numericRevision", "bootstrapContentSha256", "files");
        if ((string)saved["schema"] != "lostark.numeric-balance-save" || !IsHash((string)saved["numericRevision"]) || !IsHash((string)saved["bootstrapContentSha256"]))
            throw new IOException("Invalid numeric save receipt identity.");
        foreach (object item in (System.Collections.IEnumerable)saved["files"])
        {
            var row = (Dictionary<string, object>)item; ExactKeys(row, "path", "sha256");
            string relative = (string)row["path"], hash = (string)row["sha256"];
            if (!MutableNumericFiles.Contains(relative) || !IsHash(hash) || hashes.ContainsKey(relative) || Hash(BundlePath(relative)) != hash)
                throw new IOException("Numeric save hash/path mismatch: " + relative);
            hashes.Add(relative, hash);
        }
        if (!hashes.ContainsKey(Bootstrap) || !hashes.ContainsKey(ActiveReceipt))
            throw new IOException("Numeric save is missing its active Server generation.");
        var active = ReadObject(ActiveReceipt);
        ExactKeys(active, "schema", "parentGameplayRevision", "bootstrapContentSha256", "numericRevision", "nonNumericRowsSha256");
        if ((string)active["schema"] != "lostark.numeric-balance-active" ||
            !IsHash((string)active["parentGameplayRevision"]) || !IsHash((string)active["nonNumericRowsSha256"]) ||
            (string)active["numericRevision"] != (string)saved["numericRevision"] ||
            (string)active["bootstrapContentSha256"] != (string)saved["bootstrapContentSha256"] ||
            hashes[Bootstrap] != (string)active["bootstrapContentSha256"])
            throw new IOException("Numeric active/save generation mismatch.");
        hashes.Add(SaveReceipt, Hash(BundlePath(SaveReceipt)));
        return hashes;
    }

    private static Dictionary<string, object> ValidatePackage()
    {
        var manifest = ReadObject("bundle-manifest.json");
        if ((string)manifest["schema"] != "lostark.portable-runtime-bundle" || Convert.ToInt32(manifest["formatVersion"]) != 1 ||
            (string)manifest["configuration"] != "Release" || (string)manifest["resourcePolicy"] != "external-only-no-install" ||
            Convert.ToInt32(manifest["protocol"]) != BundleContract.Protocol || (string)manifest["serverEndpoint"] != BundleContract.Endpoint)
            throw new IOException("Incompatible or mixed package manifest. Extract the complete latest ZIP.");
        var numeric = NumericOverrides();
        var seen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (object item in (System.Collections.IEnumerable)manifest["files"])
        {
            var row = (Dictionary<string, object>)item;
            string relative = (string)row["path"];
            if (!seen.Add(relative)) throw new IOException("Duplicate package path: " + relative);
            string path = BundlePath(relative);
            if (relative.Split('/').Any(part => part.Equals("Resources", StringComparison.OrdinalIgnoreCase)))
                throw new IOException("Resources must remain external to this runtime package.");
            string savedHash;
            bool hasNumericSave = numeric.TryGetValue(relative, out savedHash);
            if (!File.Exists(path) || (!hasNumericSave && new FileInfo(path).Length != Convert.ToInt64(row["bytes"])) ||
                !String.Equals(Hash(path), hasNumericSave ? savedHash : (string)row["sha256"], StringComparison.OrdinalIgnoreCase))
                throw new IOException("Missing or damaged package file: " + relative);
        }
        foreach (string required in new[] { "Client/Bin/Release/Client.exe", "Client/Bin/Release/Engine.dll", "Server/Bin/Release/Server.exe",
            "Client/Bin/Release/vcruntime140.dll", "Server/Bin/Release/vcruntime140.dll", "Data/Effects/EffectCatalog.json",
            "Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap" })
            if (!seen.Contains(required)) throw new IOException("Manifest omits required runtime input: " + required);
        var revisions = (Dictionary<string, object>)manifest["dataRevisions"];
        ulong action = Convert.ToUInt64(revisions["sourceRevision"]), sequence = Convert.ToUInt64(revisions["sequenceRevision"]);
        if (action == 0 || sequence == 0 ||
            Revision("Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json", "revision") != action ||
            Revision("Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json", "sourceRevision") != action ||
            Revision("Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json", "sourceRevision") != action ||
            Revision("Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json", "revision") != sequence)
            throw new IOException("Authored and published Kouku revisions differ.");
        bool product = false; int gates = 0;
        foreach (string line in File.ReadLines(BundlePath("Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap")))
        {
            string[] fields = line.Split('\t');
            if (fields.Length == 4 && fields[0] == "KOUKUSAYDONPRODUCTREVISION" && fields[1] == "ENCOUNTER_KAKULSAYDON_G1")
            { if (Convert.ToUInt64(fields[3]) != action) throw new IOException("Server Action revision mismatch."); product = true; }
            if (fields.Length > 5 && fields[0] == "RAIDGATE" && fields[1] == "ENCOUNTER_KAKULSAYDON_G1")
            { if (Convert.ToUInt64(fields[5]) != sequence) throw new IOException("Server Sequence revision mismatch."); ++gates; }
        }
        if (!product || gates != 4) throw new IOException("Server raid generation is incomplete.");
        foreach (string directory in new[] { "Data", "Client/Default", "Server/Default" })
            if (!Directory.Exists(BundlePath(directory))) throw new IOException("Missing package directory: " + directory);
        return manifest;
    }

    private static string ResourceRoot(string selected)
    {
        string full = Path.GetFullPath(selected);
        foreach (string candidate in new[] { Path.Combine(full, "Client", "Bin", "Resources"), Path.Combine(full, "Resources"), full })
            if (ResourceFolders.All(name => Directory.Exists(Path.Combine(candidate, name)))) return candidate;
        throw new IOException("Select the latest Resources folder or the LostArk folder containing it. Required folders: " + String.Join(", ", ResourceFolders));
    }

    private static ProcessStartInfo ChildStartInfo(bool server, string resourceRoot)
    {
        var start = new ProcessStartInfo(BundlePath(server ? "Server/Bin/Release/Server.exe" : "Client/Bin/Release/Client.exe")) {
                UseShellExecute = false, WorkingDirectory = BundlePath(server ? "Server/Default" : "Client/Default"),
                Arguments = server ? "--bind-address 0.0.0.0" : ""
            };
        start.EnvironmentVariables["LOSTARK_PROJECT_DATA_ROOT"] = BundlePath("Data");
        start.EnvironmentVariables["LOSTARK_SERVER_DATA_ROOT"] = BundlePath("Server/Bin/DataFiles");
        start.EnvironmentVariables["LOSTARK_SERVER_HOST"] = BundleContract.Host;
        if (resourceRoot != null) start.EnvironmentVariables["LOSTARK_RESOURCE_ROOT"] = resourceRoot;
        return start;
    }

    private static Dictionary<string, object> Receipt(string resources)
    {
        var serverStart = ChildStartInfo(true, null);
        return new Dictionary<string, object> {
            { "status", "PASS" }, { "clientStarted", false }, { "serverStarted", false }, { "bundleRoot", Root },
            { "clientExecutable", BundlePath("Client/Bin/Release/Client.exe") }, { "serverExecutable", BundlePath("Server/Bin/Release/Server.exe") },
            { "clientWorkingDirectory", BundlePath("Client/Default") }, { "serverWorkingDirectory", BundlePath("Server/Default") },
            { "projectDataRoot", BundlePath("Data") }, { "serverDataRoot", serverStart.EnvironmentVariables["LOSTARK_SERVER_DATA_ROOT"] }, { "resourceRoot", resources }, { "serverEndpoint", BundleContract.Endpoint },
            { "protocol", BundleContract.Protocol }, { "resourcePolicy", "external-only-no-install" }
        };
    }

    [STAThread]
    private static int Main(string[] args)
    {
        bool checking = args.Length > 0 && (args[0] == "--check" || args[0] == "--check-package");
        string receiptPath = args.Length == 3 && args[0] == "--check" ? args[2] :
            args.Length == 2 && args[0] == "--check-package" ? args[1] : null;
        try
        {
            if (args.Length > 0 && !((args.Length == 3 && args[0] == "--check") ||
                (args.Length == 2 && args[0] == "--check-package") || (args.Length == 1 && args[0] == "--server")))
                throw new ArgumentException("Usage: LostArk.exe [--check <Resources-or-LostArk-folder> <receipt.json> | --check-package <receipt.json> | --server]");
            ValidatePackage();
            if (checking)
            {
                string resources = args[0] == "--check" ? ResourceRoot(args[1]) : null;
                File.WriteAllText(Path.GetFullPath(receiptPath), Json.Serialize(Receipt(resources)), Utf8);
                return 0;
            }
            bool server = args.Length == 1 && args[0] == "--server";
            string resourceRoot = null;
            if (!server)
            {
                using (var picker = new FolderBrowserDialog())
                {
                    picker.Description = "최신 Resources가 있는 LostArk 폴더 또는 Resources 폴더를 선택하세요.";
                    picker.ShowNewFolderButton = false;
                    if (picker.ShowDialog() != DialogResult.OK) return 0;
                    resourceRoot = ResourceRoot(picker.SelectedPath);
                }
            }
            var start = ChildStartInfo(server, resourceRoot);
            using (var process = Process.Start(start))
            {
                if (process == null) throw new IOException("The game process did not start.");
                if (process.WaitForExit(10000) && process.ExitCode != 0)
                    throw new IOException("The game exited during startup (" + process.ExitCode + "). Inspect Client/Default logs or collect diagnostics.");
            }
            return 0;
        }
        catch (Exception error)
        {
            if (checking)
            {
                if (receiptPath != null) File.WriteAllText(Path.GetFullPath(receiptPath), Json.Serialize(new {
                    status = "FAIL", clientStarted = false, serverStarted = false, error = error.Message }), Utf8);
            }
            else MessageBox.Show(error.Message, "LostArk", MessageBoxButtons.OK, MessageBoxIcon.Error);
            return 1;
        }
    }
}
