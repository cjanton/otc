#include "framework.h"
#include "headers/runtime/RuntimeHandler.h"

/**
 *
 *   ---------------- o
 * | How it's works ? |
 * o ----------------
 *
 *                           |---------------|
 *                    +++++> |  Relocations  | ======+
 * |---------|        |      |---------------|       |        |-------------------------------------------|         |---------------|       |---------------------------|
 * | Segment | =======+                              -======> | Reconstruct hotpoints with new base addr  | ======> | Own functions | ====> | Call original entry point |
 * |---------|        |      |---------------|       |        |-------------------------------------------|         |---------------|       |---------------------------|
 *                    +++++> |    Imports    | ======+
 *                           |---------------|
 *
 */

//Scary but necessary.
DWORD Segment::UnsafeAllocatedPointer = 0x0;
DWORD Segment::UnsafeLibraryPointer = 0x0;
SegmentFramework::oVirtualFunctionCaller SegmentFramework::OriginalVirtualCaller = 0x0;

BOOL APIENTRY DllMain (HMODULE module, DWORD callReason, LPVOID lpReserved) {

    //Developed by, ah cmon, u know who im. 2020.
    
    if (callReason == DLL_PROCESS_ATTACH) {

        Segment segment;
        Logger logger (Logger::LoggerType::CONSOLE);
        RuntimeHandler runtime (segment);

        PanicUtils::SetImportant (&Segment::UnsafeLibraryPointer, reinterpret_cast<DWORD> (module));

        logger.Space (2);

        logger.Info ("| [~] Performing basic checks...");

        //Do not even ask about win server :D
        if (Utils::IsWinXPOrLater () || Utils::IsWinServBuild ()) {
           logger.Info ("| [-] I cannot run OTC on the current version of Windows.");
           logger.Info ("| [~] Please upgrade to 7 or 8, or 10. (PC build)");
           return FALSE;
        }

        logger.Info ("| [+] Your windows version meets the requirements for running OTC.");
        
        //Pretty common issue.
        if (!Utils::IsRedistPackagePresent ()) {
            logger.Info ("| [!] Could not find the msvc redist package installed on your system.");
            logger.Info ("| [~] You can download and install the redist build from here: https://www.microsoft.com/en-us/download/confirmation.aspx?id=52685.");
            return FALSE;
        }

        logger.Info ("| [+] Found the required msvc redist build on your system.");
        logger.Info ("| [+] All checks passed.");

        logger.Space ();

        logger.Info ("| [~] Extracting segment to memory....");

        runtime.ExtractSegment ();
        
        logger.Info ("| [~] Reconstructing hot-points...");

        runtime.ReconstructHotPoints ();
        
        logger.Info ("| [+] Wait until the framework complete routine work...");

        logger.Space ();

        logger.Info ("| [~] Filling the dependency table... (~7-15 sec)");

        segment.GetFramework().CreateDependencyTable ();

        logger.Info ("| [~] Updating netvars...");

        segment.GetFramework().UpdateNetVars ();

        logger.Info ("| [~] Install hook for virtual function caller...");

        segment.GetFramework().InstallHook ();

        logger.Info ("| [~] Updating watermark...");

        segment.GetFramework().UpdateWatermark (" ☁ ", " ☁ ");
        segment.GetFramework().UpdateMenuWatermark (" ☁ ");

        logger.Space ();

        logger.Info ("| [~] Invoking OEP...");
       
        //Make segment alive.
        runtime.InvokeOEP ();

        //Hide menu for better log look.
        segment.GetFramework().SetMenuStatus (false);

        logger.Info ("| [+] All loader procedures finished. Gl.");

        logger.Space ();

    }

    return TRUE;

}