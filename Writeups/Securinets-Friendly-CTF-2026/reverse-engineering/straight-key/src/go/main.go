package main

import (
	"bytes"
	"debug/pe"
	"errors"
	"fmt"
	"os"
	"runtime"
	"runtime/debug"
	"syscall"
	"time"
)

const (
	vkTarget   = 0x4B
	passes     = numSlots
	passRest   = 3000
	lingerSecs = 20
)

var (
	kernel32  = syscall.NewLazyDLL("kernel32.dll")
	procTicks = kernel32.NewProc("GetTickCount")
)

var errUnavailable = errors.New("self-test unavailable on this system")

type session struct {
	state    uint32
	seed     uint32
	send     uintptr
	rejected uint32
}

type stage func(*session) error

func schedule() []stage {
	return []stage{
		(*session).announce,
		(*session).derive,
		(*session).deliver,
		(*session).cleanup,
	}
}

func main() {
	runtime.LockOSThread()
	debug.SetGCPercent(-1)

	s := &session{}
	for _, run := range schedule() {
		if err := run(s); err != nil {
			fmt.Println(err)
			linger()
			os.Exit(1)
		}
	}
}

func next(state *uint32) byte {
	x := *state
	x ^= x << 13
	x ^= x >> 17
	x ^= x << 5
	*state = x
	return byte((x >> 16) & 0xFF)
}

func (s *session) next() byte { return next(&s.state) }

//go:noinline
func stored() (candidates, message []byte) {
	return patStore[patMarkLen:], msgBlob[:]
}

func (s *session) announce() error {
	_, blob := stored()
	line := make([]byte, len(blob))
	s.state = msgSeed
	for i := range line {
		line[i] = blob[i] ^ s.next()
	}
	fmt.Println(string(line))
	wipeBytes(line)
	return nil
}

//go:noinline
func (s *session) derive() error {
	path, err := os.Executable()
	if err != nil {
		return errUnavailable
	}
	raw, err := os.ReadFile(path)
	if err != nil {
		return errUnavailable
	}
	image, err := pe.NewFile(bytes.NewReader(raw))
	if err != nil {
		return errUnavailable
	}
	defer image.Close()

	sec := image.Section(".text")
	if sec == nil {
		return errUnavailable
	}
	lo, hi := int(sec.Offset), int(sec.Offset)+int(sec.Size)
	if lo < 0 || hi > len(raw) || lo > hi {
		return errUnavailable
	}

	h := uint32(2166136261)
	for _, b := range raw[lo:hi] {
		h = (h ^ uint32(b)) * 16777619
	}
	s.seed = h
	return nil
}

func (s *session) slotSeed(slot int) uint32 { return s.seed ^ slotSalt[slot] }

func tick() uint32 {
	if err := procTicks.Find(); err != nil {
		return uint32(time.Now().UnixMilli())
	}
	r, _, _ := syscall.SyscallN(procTicks.Addr())
	return uint32(r)
}

func (s *session) cleanup() error {
	s.seed, s.state, s.send, s.rejected = 0, 0, 0, 0
	return nil
}

func linger() { time.Sleep(lingerSecs * time.Second) }

func wipeBytes(b []byte) {
	for i := range b {
		b[i] = 0
	}
}
