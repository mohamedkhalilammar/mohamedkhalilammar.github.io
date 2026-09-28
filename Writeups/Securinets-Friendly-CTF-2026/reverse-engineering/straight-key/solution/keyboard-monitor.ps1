<#
    REFERENCE SOLUTION -- do not ship to players. This is the tool the challenge exists
    to make them write; it lives here so we can prove the challenge is solvable, and so a
    table that is genuinely stuck can be helped at the venue.

    A keyboard event monitor. It watches every key on the keyboard, and for each one
    reports when it went down, when it came up, and how long it was held.

    Start this FIRST, then run the sample in a second window.

        # watch everything, human readable
        powershell -ExecutionPolicy Bypass -File keyboard-monitor.ps1 -Seconds 200

        # capture in the format the decoder eats
        powershell -ExecutionPolicy Bypass -File keyboard-monitor.ps1 -Seconds 200 `
                   -OnlyKey 0x4B -Log > capture.txt
        python3 ../tools/decode_log.py capture.txt

    It only ever READS key state. It presses nothing and sends nothing anywhere.
#>
[CmdletBinding()]
param(
    [int]    $Seconds = 200,
    [int]    $OnlyKey = 0,     # 0 = every key; otherwise a virtual-key code, e.g. 0x4B
    [switch] $Log              # emit "<seconds> DOWN|UP" instead of the readable table
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# GetAsyncKeyState reads the live key state directly. It needs no message pump, which
# matters -- a console script has no window, so the message-queue based GetKeyboardState
# would sit there returning stale data forever.
#
# The scan across all 255 keys is done inside C# rather than in a PowerShell loop; doing
# 255 P/Invoke calls per sample from PowerShell is slow enough to blur short presses.
Add-Type -Namespace Win32 -Name Keys -MemberDefinition @'
[DllImport("user32.dll")]
private static extern short GetAsyncKeyState(int vKey);

public static byte[] Snapshot() {
    byte[] state = new byte[256];
    for (int vk = 1; vk < 256; vk++) {
        state[vk] = (byte)(((GetAsyncKeyState(vk) & 0x8000) != 0) ? 1 : 0);
    }
    return state;
}
'@

function Get-KeyName {
    param([int] $Vk)
    # Letters and digits render as themselves; everything else stays as its code, which
    # is honest -- guessing at names for 200 keys would invent more than it explains.
    if (($Vk -ge 0x30 -and $Vk -le 0x39) -or ($Vk -ge 0x41 -and $Vk -le 0x5A)) {
        return [string][char] $Vk
    }
    return ('VK_0x{0:X2}' -f $Vk)
}

$sw       = [System.Diagnostics.Stopwatch]::StartNew()
$previous = [Win32.Keys]::Snapshot()
$downAt   = @{}                  # vk -> seconds at which it went down
$counts   = @{}                  # vk -> how many complete presses seen
$samples  = 0

if (-not $Log) {
    Write-Host "watching the keyboard for $Seconds seconds -- start the sample now" -ForegroundColor Cyan
    Write-Host ''
    Write-Host ('{0,10}  {1,-8}  {2,-6}  {3}' -f 'time', 'key', 'event', 'held')
    Write-Host ('{0,10}  {1,-8}  {2,-6}  {3}' -f '----', '---', '-----', '----')
}

while ($sw.Elapsed.TotalSeconds -lt $Seconds) {
    $now = [Win32.Keys]::Snapshot()
    $samples++
    $t = $sw.Elapsed.TotalSeconds

    for ($vk = 1; $vk -lt 256; $vk++) {
        if ($now[$vk] -eq $previous[$vk]) { continue }
        if ($OnlyKey -ne 0 -and $vk -ne $OnlyKey) { continue }

        if ($now[$vk] -eq 1) {
            $downAt[$vk] = $t
            if ($Log) { '{0:F3} DOWN' -f $t }
            else { Write-Host ('{0,10:F3}  {1,-8}  {2,-6}' -f $t, (Get-KeyName $vk), 'down') }
        }
        else {
            $held = if ($downAt.ContainsKey($vk)) { ($t - $downAt[$vk]) * 1000 } else { -1 }
            $counts[$vk] = 1 + $(if ($counts.ContainsKey($vk)) { $counts[$vk] } else { 0 })
            if ($Log) { '{0:F3} UP' -f $t }
            else {
                Write-Host ('{0,10:F3}  {1,-8}  {2,-6}  {3,6:F0} ms' -f `
                    $t, (Get-KeyName $vk), 'up', $held)
            }
        }
    }

    $previous = $now
    Start-Sleep -Milliseconds 5
}

if (-not $Log) {
    Write-Host ''
    Write-Host ('sampled {0} times in {1:F0}s (~{2:F0} ms per sample)' -f `
        $samples, $sw.Elapsed.TotalSeconds, ($sw.Elapsed.TotalMilliseconds / $samples))
    if ($counts.Count -eq 0) {
        Write-Host 'no keys fired.' -ForegroundColor Yellow
    }
    else {
        Write-Host ''
        Write-Host 'keys seen:'
        foreach ($vk in ($counts.Keys | Sort-Object)) {
            Write-Host ('  {0,-8} {1} presses' -f (Get-KeyName $vk), $counts[$vk])
        }
    }
}
