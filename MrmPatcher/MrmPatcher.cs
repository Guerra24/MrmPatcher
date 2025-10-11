/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

using System.Runtime.InteropServices;

namespace MrmPatcher;

internal static partial class MrmPatcherMethods
{
    [LibraryImport("MrmPatcher", EntryPoint = "PatchMrm")]
    public static partial void PatchMrm(byte[] data, long length);

    [LibraryImport("MrmPatcher", EntryPoint = "UnpatchMrm")]
    public static partial void UnpatchMrm();
}

public class MrmPatcher : IDisposable
{

    public MrmPatcher(byte[] data, long length)
    {
        MrmPatcherMethods.PatchMrm(data, length);
    }

    public MrmPatcher(Stream stream)
    {
        var data = new byte[stream.Length];
        stream.ReadExactly(data);
        MrmPatcherMethods.PatchMrm(data, data.Length);
    }

    public void Dispose()
    {
        MrmPatcherMethods.UnpatchMrm();
    }
}