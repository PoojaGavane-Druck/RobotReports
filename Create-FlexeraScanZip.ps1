# This Powershell script will create a repeatable zip file of the PV624 project to import to Flexera FlexNet Code Insight https://flexera-oss.na01.ibhge.com/codeinsight/Login
# Intended to be called from a batch file with same filename
# Zip file should be tested that it is complete and can be used to create a clean build from sources
# Simon Smith 30/10/19 modified by Pooja gavane and Nageswara Rao

Clear-Host

# Change to project repo root folder
$scriptpath = $MyInvocation.MyCommand.Path
$dir = Split-Path $scriptpath
#$repo = $dir | Split-Path -Parent
$repo = $dir 
Set-Location $repo
$hash = git log -1 --format="%H"
$zipFileName = "{0}\PV624_MainApp_Src_{1}_{2}.zip" -f $repo, (Date -Format "yyyy-MM-dd"), ($hash)
$excludedRegex = "CMSIS\\NN|CMSIS\\DSP|DPI705E_HW|touchgfx_backup|Python Control Code|XComp_BL652_8229_513C.exe|UwTerminalX.exe|Style|Programming|LDRA|Tools|Programming.zip|CITests|Python|Create-FlexeraScanZip.bat|Create-FlexeraScanZip.ps1"

"Scanning folder '{0}'" -f $repo

# Create list of files used in build to zip
$sourceFiles = Get-ChildItem $repo -Recurse -Include *.c, *.sb, *.uwc, *.cpp, *.h, *.hpp, *.s, *.asm, *.icf, *.eww, *.ewp, *.py, Jenkinsfile, *.bat, *.chm, *.html, *.txt, *.exe, *.md, *.wsdt, *.ewd, *.ps1, *.bat, *.otf, *.ttf, *.bmp, *.png, *.ioc, *.touchgfx, texts.xlsx, touchgfx_core.a | ? {$_.FullName -notmatch $excludedRegex}
"{0} files found" -f $sourceFiles.Count

# Create/replace existing zip file
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
Remove-Item $zipFileName -ErrorAction SilentlyContinue
$zip = [System.IO.Compression.ZipFile]::Open($zipFileName, [System.IO.Compression.ZipArchiveMode]::Create)

# Write entries with relative paths as names (ignore export PC app as not part of the project)
foreach ($fname in $sourceFiles) `
{
    $rname = $(Resolve-Path -Path $fname -Relative) -replace '\.\\',''
    echo $rname
    $zentry = $zip.CreateEntry($rname)
    $zentryWriter = New-Object -TypeName System.IO.BinaryWriter $zentry.Open()
    $zentryWriter.Write([System.IO.File]::ReadAllBytes($fname))
    $zentryWriter.Flush()
    $zentryWriter.Close()
}

# Clean up
Get-Variable -exclude Runspace | Where-Object {$_.Value -is [System.IDisposable]} | Foreach-Object `
{
    $_.Value.Dispose();
    Remove-Variable $_.Name -ErrorAction SilentlyContinue
};

"`nCreated '{0}' containing {1} source files" -f $zipFileName, $sourceFiles.Count
