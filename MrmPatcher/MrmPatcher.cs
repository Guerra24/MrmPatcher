/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;

namespace MrmPatcher;

internal static partial class MrmPatcherMethods
{
    [LibraryImport("MrmPatcher", EntryPoint = "PatchMrm")]
    public static partial void PatchMrm(byte[] data, long length);

    [LibraryImport("MrmPatcher", EntryPoint = "UnpatchMrm")]
    public static partial void UnpatchMrm();
}

public class MrmPatcherHelper : IDisposable
{

    [FeatureSwitchDefinition("MrmPatcher.IsEnabled")]
    private static bool IsEnabled => AppContext.TryGetSwitch("MrmPatcher.IsEnabled", out bool isEnabled) && isEnabled;

    public MrmPatcherHelper(byte[] data)
    {
        if (IsEnabled)
            MrmPatcherMethods.PatchMrm(data, data.LongLength);
    }

    public MrmPatcherHelper(Stream stream)
    {
        if (IsEnabled)
        {
            var data = new byte[stream.Length];
            stream.ReadExactly(data);
            MrmPatcherMethods.PatchMrm(data, data.Length);
        }
    }

    public void Dispose()
    {
        if (IsEnabled)
            MrmPatcherMethods.UnpatchMrm();
    }
}