using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using JetBrains.Annotations;

namespace GodotTools.Utils
{
    [SuppressMessage("ReSharper", "InconsistentNaming")]
    public static class OS
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern string GetPlatformName();

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern bool UnixFileHasExecutableAccess(string filePath);

        public static class Names
        {
            public const string Windows = "Windows";
            public const string OSX = "OSX";
            public const string X11 = "X11";
            public const string Server = "Server";
            public const string UWP = "UWP";
            public const string Haiku = "Haiku";
            public const string Android = "Android";
            public const string iOS = "iOS";
            public const string HTML5 = "HTML5";
        }

        public static class Platforms
        {
            public const string Windows = "windows";
            public const string OSX = "osx";
            public const string X11 = "x11";
            public const string Server = "server";
            public const string UWP = "uwp";
            public const string Haiku = "haiku";
            public const string Android = "android";
            public const string iOS = "iphone";
            public const string HTML5 = "javascript";
        }

        public static readonly Dictionary<string, string> PlatformNameMap = new Dictionary<string, string>
        {
            [Names.Windows] = Platforms.Windows,
            [Names.OSX] = Platforms.OSX,
            [Names.X11] = Platforms.X11,
            [Names.Server] = Platforms.Server,
            [Names.UWP] = Platforms.UWP,
            [Names.Haiku] = Platforms.Haiku,
            [Names.Android] = Platforms.Android,
            [Names.iOS] = Platforms.iOS,
            [Names.HTML5] = Platforms.HTML5
        };

        private static bool IsOS(string name)
        {
            return name.Equals(GetPlatformName(), StringComparison.OrdinalIgnoreCase);
        }

        private static bool IsAnyOS(IEnumerable<string> names)
        {
            return names.Any(p => p.Equals(GetPlatformName(), StringComparison.OrdinalIgnoreCase));
        }

        private static readonly Lazy<bool> _isWindows = new Lazy<bool>(() => IsOS(Names.Windows));
        private static readonly Lazy<bool> _isOSX = new Lazy<bool>(() => IsOS(Names.OSX));
        private static readonly Lazy<bool> _isX11 = new Lazy<bool>(() => IsOS(Names.X11));
        private static readonly Lazy<bool> _isServer = new Lazy<bool>(() => IsOS(Names.Server));
        private static readonly Lazy<bool> _isUWP = new Lazy<bool>(() => IsOS(Names.UWP));
        private static readonly Lazy<bool> _isHaiku = new Lazy<bool>(() => IsOS(Names.Haiku));
        private static readonly Lazy<bool> _isAndroid = new Lazy<bool>(() => IsOS(Names.Android));
        private static readonly Lazy<bool> _isiOS = new Lazy<bool>(() => IsOS(Names.iOS));
        private static readonly Lazy<bool> _isHTML5 = new Lazy<bool>(() => IsOS(Names.HTML5));
        private static readonly Lazy<bool> _isUnixLike = new Lazy<bool>(() => IsAnyOS(UnixLikePlatforms));

        public static bool IsWindows => _isWindows.Value || IsUWP;
        public static bool IsOSX => _isOSX.Value;
        public static bool IsX11 => _isX11.Value;
        public static bool IsServer => _isServer.Value;
        public static bool IsUWP => _isUWP.Value;
        public static bool IsHaiku => _isHaiku.Value;
        public static bool IsAndroid => _isAndroid.Value;
        public static bool IsiOS => _isiOS.Value;
        public static bool IsHTML5 => _isHTML5.Value;

        private static readonly string[] UnixLikePlatforms = {Names.OSX, Names.X11, Names.Server, Names.Haiku, Names.Android, Names.iOS};

        public static bool IsUnixLike => _isUnixLike.Value;

        public static char PathSep => IsWindows ? ';' : ':';

        public static string PathWhich([NotNull] string name)
        {
            return IsWindows ? PathWhichWindows(name) : PathWhichUnix(name);
        }

        private static string PathWhichWindows([NotNull] string name)
        {
            string[] windowsExts = Environment.GetEnvironmentVariable("PATHEXT")?.Split(PathSep) ?? new string[] { };
            string[] pathDirs = Environment.GetEnvironmentVariable("PATH")?.Split(PathSep);
            char[] invalidPathChars = Path.GetInvalidPathChars();

            var searchDirs = new List<string>();

            if (pathDirs != null)
            {
                foreach (var pathDir in pathDirs)
                {
                    if (pathDir.IndexOfAny(invalidPathChars) != -1)
                        continue;

                    searchDirs.Add(pathDir);
                }
            }

            string nameExt = Path.GetExtension(name);
            bool hasPathExt = !string.IsNullOrEmpty(nameExt) && windowsExts.Contains(nameExt, StringComparer.OrdinalIgnoreCase);

            searchDirs.Add(System.IO.Directory.GetCurrentDirectory()); // last in the list

            if (hasPathExt)
                return searchDirs.Select(dir => Path.Combine(dir, name)).FirstOrDefault(File.Exists);

            return (from dir in searchDirs
                select Path.Combine(dir, name)
                into path
                from ext in windowsExts
                select path + ext).FirstOrDefault(File.Exists);
        }

        private static string PathWhichUnix([NotNull] string name)
        {
            string[] pathDirs = Environment.GetEnvironmentVariable("PATH")?.Split(PathSep);
            char[] invalidPathChars = Path.GetInvalidPathChars();

            var searchDirs = new List<string>();

            if (pathDirs != null)
            {
                foreach (var pathDir in pathDirs)
                {
                    if (pathDir.IndexOfAny(invalidPathChars) != -1)
                        continue;

                    searchDirs.Add(pathDir);
                }
            }

            searchDirs.Add(System.IO.Directory.GetCurrentDirectory()); // last in the list

            return searchDirs.Select(dir => Path.Combine(dir, name))
                .FirstOrDefault(path => File.Exists(path) && UnixFileHasExecutableAccess(path));
        }

        public static void RunProcess(string command, IEnumerable<string> arguments)
        {
            // TODO: Once we move to .NET Standard 2.1 we can use ProcessStartInfo.ArgumentList instead
            string CmdLineArgsToString(IEnumerable<string> args)
            {
                // Not perfect, but as long as we are careful...
                return string.Join(" ", args.Select(arg => arg.Contains(" ") ? $@"""{arg}""" : arg));
            }

            var startInfo = new ProcessStartInfo(command, CmdLineArgsToString(arguments))
            {
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                UseShellExecute = false
            };

            using (Process process = Process.Start(startInfo))
            {
                if (process == null)
                    throw new Exception("No process was started");

                process.BeginOutputReadLine();
                process.BeginErrorReadLine();
                if (IsWindows && process.Id > 0)
                    User32Dll.AllowSetForegroundWindow(process.Id); // allows application to focus itself
            }
        }

        public static int ExecuteCommand(string command, IEnumerable<string> arguments)
        {
            // TODO: Once we move to .NET Standard 2.1 we can use ProcessStartInfo.ArgumentList instead
            string CmdLineArgsToString(IEnumerable<string> args)
            {
                // Not perfect, but as long as we are careful...
                return string.Join(" ", args.Select(arg => arg.Contains(" ") ? $@"""{arg}""" : arg));
            }

            var startInfo = new ProcessStartInfo(command, CmdLineArgsToString(arguments));

            Console.WriteLine($"Executing: \"{startInfo.FileName}\" {startInfo.Arguments}");

            // Print the output
            startInfo.RedirectStandardOutput = false;
            startInfo.RedirectStandardError = false;

            startInfo.UseShellExecute = false;

            using (var process = new Process {StartInfo = startInfo})
            {
                process.Start();
                process.WaitForExit();

                return process.ExitCode;
            }
        }
    }
}
