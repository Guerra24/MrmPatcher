# MrmPatcher

[![NuGet](https://img.shields.io/nuget/v/MrmPatcher)](https://www.nuget.org/packages/MrmPatcher/)

Based on the [SingleExeXamlIsland](https://github.com/ysc3839/SingleExeXamlIsland) project but instead for C# Native AOT.

This nuget allows publishing unpackaged UWP Xaml Islands apps fully self-contained without additional files.

Simply wrap any calls to `WindowsXamlManager.InitializeForCurrentThread()` with `MrmPatcherHelper` and pass the content of `resources.pri`.

```cs
using (new MrmPatcherHelper(stream or byte array))
{
	windowsXamlManager = WindowsXamlManager.InitializeForCurrentThread();
}
```

## Projects

### MrmPatcher

C# helper and interop with the native library, also automates the linking of the native project when publishing NAOT binaries.

### MrmPatcher.Native

A C++ static library that handles the patching. Uses Detours to patch the respective Win32 APIs that are used by Xaml.
