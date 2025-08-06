## Editor
The recent HND files can be opened in HelpNDoc v9.x.

You can download the HelpNDoc v9 Personal Edition for free without registration [here](https://www.helpndoc.com/download/), but note there is a footer added to all pages in output files.  
(Phi has a paid-for copy, bought by Patreon money.)

If for some reason you wish to open LB's original HelpNDoc v2.x file, you can download HelpNDoc v3.9 from [here](https://dl.ibe-software.com/hnd/helpndoc-setup-3.9.1.648.exe).  
(HelpNDoc v2 is not available for download, but v3 can open v2 files)

The flowchart file is made on [draw.io](https://draw.io/) and can be uploaded, edited and downloaded there.
(Note the site was rebranded to [diagrams.net](https://app.diagrams.net/).

## Editing notes
If you are editing, please consider adding alt text to any pictures you add (under Picture > Format), for accessibility.  
There is a grey border of 2px around nearly all images, to cleanly separate the background of the images from the pages.

There are more notes on editing included in the FOR EDITORS topic in the HND file itself. This should be the last or first topic in the file.

You **cannot** build CHMs to #MFAs and Documentation. This is because the '#' makes the CHM generator fail; it will create a CHM without any error, but each page in it will not display when opened.  
Instead, target the MFX folder, or wherever else does not have that character.

Don't forget, even if it does build correctly, you must have the CHM unblocked in file properties. To do so, right-click the generated file in Windows Explorer, select Properties, and there should be an Unblock button.  
This is part of the Zone.Identifier, a rather pointless way of classifying the source of things as from Internet, LAN, or so on... pointless as antiviruses scan new files regardless of source.  
You can disable Zone Identifier for all new downloads using the instructions [here](https://www.google.com/search?q=zone+identifier).
