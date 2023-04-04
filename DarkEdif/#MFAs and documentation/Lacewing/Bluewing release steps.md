Bluewing release steps
====

This list implies bluewing-cpp-server has an edit, and MMF2Exts needs it, although the process is similar for reverse;
except that MMF2Exts does not always need any changes to support new builds. For example, an A/C/E fix may not affect
the C++ standalone bluewing-cpp-server.

Only Phi is expected to go through all these steps; other contributers likely won't have the tools/logins to do most
of these.

Time estimation for a release: 30 minutes minimum – timed it once.

Steps
---- 
1. Add DarkEdif ext DB entry for new builds.
2. Update lacewing buildnum in Lacewing.h for Client and Server.
3. Sync bluewing-cpp-server with MMF2Exts.
4. Confirm WSL Linux build server is running (WSL + openssh)
5. Build all Linux and Windows distributions.  
   * Check for warnings/errors; if any, fix and restart release.  
6. Test Linux and Windows. Test release and debug, with Windows and HTML5 clients.
7. Next, build MMF2Exts.
8. Build Client and Server. Test both in Fusion to a reasonable extent.  
   * Repeat test in release builds.  
   * Delete MMF2Exts\Temp\Bluewing folders to ensure fresh build, and do a Rebuild of all platforms. Note build time.  
     (You may have to repeat build for iOS/Mac so all build configs visibly succeed.)  
   * Test both objects in Windows, Android, iOS, Mac, HTML5/UWP.  
   * If error, fix, and restart release.
9. Commit new bluewing-cpp-server, including changelog and release notes.  
   * Go over all file changes to ensure they're in commit notes and there isn't some typo or leftover debug lines.  
   * Remember if there is some edit done, copy it back to MMF2Exts.
10. Package bluewing-cpp-server Windows and Linux, both port-fixed and port-in-input (4 zips).  
	* Update build date in all ZIPs of built files.  
	* Ensure the port-in-input edit is undone.
11. Attach new zips with changelog to bluewing-cpp-server GitHub release.
12. Shut down old running server app, copy new  bluewing-cpp-server-linux to Darkwire Server 1, and reboot DS1.  
	* Confirm DS1 works. Test Windows and HTML5 clients, on both secure and insecure HTML5.
13. Check help file has revision incremented.
14. Prepare changelog of changes in help file.
15. Build help file with editor page included, for both CHM and HTML5.  
	* Confirm help file looks correct in editor page.  
	* Confirm help file has "..." and "-" replaced with "…" and "–".  
	* Confirm new images in help file have alt text for accessibility.  
16. After all is working, rebuild CHM/HTML5 with editor page removed.
17. Copy CHM from MMF2Exts Help folder to Documentations.
18. Update HTML5/UWP internal buildnum/SDK numbers.
19. Copy HTML5/UWP from Client to MFX\Data\Runtime folders. UWP is raw, not zipped.
20. ZIP the MMF2Exts Client and Server, including non-Patreon and partial versions.  
	* Update build dates, numbers and comments inside zip.
21. Commit new Lacewing help file – use changelog from above.
22. Commit new MMF2Exts version.
23. Extract new versions to local Fusion install folders - CF2.5 and MMF2.0
24. Move old ext ZIPs to local old versions folder.
25. Copy ext ZIPs to local Ext Installers folder.
26. Open Darkwire website file server
	* Move old versions from website /storage/ to /temp/ old versions folder.
	* Upload new ext zips to /storage/ and /exts/.
27. Open DarkEdif ext DB app.  
	* Check changelog has all necessary entries (for Fusion users, not C++ devs).  
	* Switch new private build to public.
28. Open extlist.php ext DB excel sheet.
	* Update excel version, build date, MFX size, platforms.
	* Save. Copy columns, save two TXT files after removing header row , and for second one, removing necessary columns.
		* Convert both TXT to CNC Array format and trim using Fusion app.
		* Upload both CNC Array to /storage/, overwriting old /storage/.
		* Delete the extlist cache from /storage/
		* Open storage extlist in browser and confirm it is not reporting error and has recent build numbers.
29. Open CEM ext uploader.
	* Copy Client/Server zip from MFX folder.
	* Remove the ANSI Windows files.
	* Manually edit CEM JSON file; update build number (x3 for server), zip size and modified date from file properties.
		* JSON date should be last modified date inside the zip; normally this is Exts\Unicode mfx file.
	* Submit new versions of both to CEM.
30. Go on CT forum, post new version with changelog.
	* Update comment of ZIPs of exts to new forum post.
		* Reupload to /storage/ and /exts/.
