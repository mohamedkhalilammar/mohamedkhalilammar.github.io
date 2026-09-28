// Emitter probe -- OUR tooling, never shipped to players.
//
// Straight Key injects keystrokes by resolving win32u.NtUserSendInput at runtime and
// calling it directly, and the process never loads user32.dll (import table is kernel32
// only). notes/design.md calls that "A1", the load-bearing assumption of the whole
// design, and records it as never verified on Windows. Wine cannot verify it: Wine's
// win32u does the work in user mode and has no dependency on user32 having initialised
// the process as a GUI client, so a clean wine capture proves nothing about real win32k.
//
// This probe settles it in 30 seconds on a real Windows VM. It injects 'k' three ways and
// prints the return value of every call, plus which window is actually in front:
//
//	A  win32u.NtUserSendInput  with user32 NEVER loaded   <- what the sample does
//	B  win32u.NtUserSendInput  after user32 is loaded     <- the one-line fix, if A is 0
//	C  user32.SendInput                                   <- the fallback, if B is 0
//
// A non-zero return means win32k accepted the event. Zero means it was dropped, which is
// exactly the reported symptom (no 'k' appears anywhere). The sample discards this value,
// which is why the failure is silent.
//
//	GOOS=windows GOARCH=amd64 go build -o ../../build/emitter-probe.exe ./tools/probe_emitter
package main

import (
	"fmt"
	"syscall"
	"time"
	"unsafe"
)

const (
	vkK        = 0x4B
	keyeventUp = 0x0002
	inputSize  = 40
	holdMs     = 300
	presses    = 3
)

func inputBuf(flags uint32) [inputSize]byte {
	var b [inputSize]byte
	*(*uint32)(unsafe.Pointer(&b[0])) = 1
	*(*uint16)(unsafe.Pointer(&b[8])) = vkK
	*(*uint32)(unsafe.Pointer(&b[12])) = flags
	return b
}

func burst(name string, addr uintptr) {
	fmt.Printf("  %s: ", name)
	for i := 0; i < presses; i++ {
		down := inputBuf(0)
		r1, _, e1 := syscall.SyscallN(addr, 1, uintptr(unsafe.Pointer(&down[0])), inputSize)
		time.Sleep(holdMs * time.Millisecond)
		up := inputBuf(keyeventUp)
		r2, _, _ := syscall.SyscallN(addr, 1, uintptr(unsafe.Pointer(&up[0])), inputSize)
		fmt.Printf("down=%d up=%d ", r1, r2)
		if r1 == 0 {
			fmt.Printf("(lasterr=%v) ", e1)
		}
		time.Sleep(200 * time.Millisecond)
	}
	fmt.Println()
}

func foreground() string {
	u32 := syscall.NewLazyDLL("user32.dll")
	gfw, gwt := u32.NewProc("GetForegroundWindow"), u32.NewProc("GetWindowTextW")
	h, _, _ := syscall.SyscallN(gfw.Addr())
	if h == 0 {
		return "(none)"
	}
	var buf [256]uint16
	syscall.SyscallN(gwt.Addr(), h, uintptr(unsafe.Pointer(&buf[0])), uintptr(len(buf)))
	return syscall.UTF16ToString(buf[:])
}

func resolve(dll, proc string) (uintptr, error) {
	p := syscall.NewLazyDLL(dll).NewProc(proc)
	if err := p.Find(); err != nil {
		return 0, err
	}
	return p.Addr(), nil
}

func main() {
	fmt.Println("Straight Key emitter probe")
	fmt.Println("Focus an empty Notepad now. Starting in 5s.")
	time.Sleep(5 * time.Second)

	w32, err := resolve("win32u.dll", "NtUserSendInput")
	if err != nil {
		fmt.Printf("\nRESOLVE FAILED: win32u.dll!NtUserSendInput -> %v\n", err)
		fmt.Println("That alone explains it: the sample prints its error to stderr and")
		fmt.Println("exits immediately, so a double-clicked console just flashes and closes.")
		fmt.Println("\nPress Enter to close.")
		fmt.Scanln()
		return
	}
	fmt.Printf("\nresolved win32u.dll!NtUserSendInput at %#x\n", w32)

	fmt.Println("A  win32u, nothing else done -- exactly what the sample does:")
	burst("A", w32)

	u32 := syscall.NewLazyDLL("user32.dll")
	gfw := u32.NewProc("GetForegroundWindow")
	if err := gfw.Find(); err != nil {
		fmt.Printf("\nRESOLVE FAILED: user32.dll!GetForegroundWindow -> %v\n", err)
		return
	}
	fmt.Println("B  win32u, user32 LOADED but never called:")
	burst("B", w32)

	h, _, _ := syscall.SyscallN(gfw.Addr())
	fmt.Printf("\nmade one user32 call: GetForegroundWindow() = %#x (%q)\n", h, foreground())
	fmt.Println("C  win32u, after one real user32 call:")
	burst("C", w32)

	send, err := resolve("user32.dll", "SendInput")
	if err == nil {
		fmt.Println("D  user32.SendInput:")
		burst("D", send)
	}

	fmt.Println("\nCheck Notepad. Each burst that worked left 3 'k' characters.")
	fmt.Println("\nPress Enter to close.")
	fmt.Scanln()
}
