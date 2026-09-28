//go:build designer

package main

import (
	"fmt"
	"strconv"
	"strings"
)

func (s *session) deliver() error {
	cand, _ := stored()
	for pass := 0; pass < numSlots; pass++ {
		off := pass * patBytes
		cipher := cand[off : off+patBytes]
		kState := s.slotSeed(pass)

		var curByte byte
		parts := make([]string, patBits)
		for i := 0; i < patBits; i++ {
			if i&7 == 0 {
				curByte = cipher[i>>3] ^ next(&kState)
			}
			bit := (curByte >> uint(7-(i&7))) & 1
			ms := patUnit
			if bit == 1 {
				ms = patUnit * 3
			}
			parts[i] = strconv.Itoa(ms)
		}
		fmt.Printf("slot=%d %s\n", pass, strings.Join(parts, ","))
	}
	return nil
}
