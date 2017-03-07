Settings, adjustments and options to take when building this configuration
==========================================================================


Environment variables
---------------------
OCAPI=[PathToOcapi]
e.g.: >set OCAPI=D:\myfiles\ocapi
The complete path to the ocapi hardware abstraction layer files
Use console commandos (SET/EXPORT) or in Eclipse the Run | External Tools | External Tools Configurations | Environment tab.


Ant script parameters (build.xml)
---------------------------------
-Dmcu=msp430x5438
Define the target machine type.
Provide as parameter when calling the ant script from command prompt or use External Tools Configurations | Main tab when in Eclipse.