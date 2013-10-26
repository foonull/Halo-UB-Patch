=== Halo Universal Binary 2.x Patch ===
(Version 1.0)

— About —
You most likely want to apply this patch because you upgraded to OS X 10.9 Mavericks and cannot run Halo anymore. This bug may not hit every system, but this could easily change in the future.

— Assumptions —
This patch assumes you have the universal binary version of Halo installed (2.x), and preferably you are on the latest official patch (version 2.0.4).

— Instructions —
First locate your Halo app in Finder. If Halo is in your dock, command clicking it will reveal the app in Finder.

Then control click (or right click) on your Halo app, show package contents, and go into Contents -> MacOS. In the MacOS folder, you will find an executable file named Halo. Rename that file to HaloUB instead.

From the Halo UB Patch folder, drag the ‘Halo’ and ‘mavericks_patch.dylib’ files into the MacOS folder you were at. Now the MacOS folder should have three files: Halo, HaloUB, and mavericks_patch.dylib

That is all. Launching the Halo app should now work.

— Technical Info (for the curious) —

The bug in Halo is that is allocating memory by calling mmap with a fixed memory address. As such, Halo was overwriting crucial data on some systems due to the malloc changes in 10.9. The fix presented is calling mmap earlier on in the process.

— Misc. Links —
HaloMD - http://halomd.net (for extending the life of Halo Mac and making it fun again)
MacGamingMods - http://macgamingmods.com/forum/ (for Halo Mac modding community)
GitHub - http://github.com/foonull/Halo-UB-Patch (for source code)