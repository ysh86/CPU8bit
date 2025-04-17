package main

import (
	"encoding/binary"
	"fmt"
	"io"
	"os"
)

type Header struct {
	Fmt []byte
	Z   []byte
	Num uint32
}

func ReadHeader(r io.Reader) (h Header, size int) {
	h.Fmt = make([]byte, 3)
	h.Z = make([]byte, 5)

	binary.Read(r, binary.LittleEndian, &h.Fmt)
	binary.Read(r, binary.LittleEndian, &h.Z)
	binary.Read(r, binary.LittleEndian, &h.Num)

	return h, len(h.Fmt) + len(h.Z) + 4
}

func (h *Header) Print() {
	println(string(h.Fmt))
	for i, z := range h.Z {
		fmt.Printf("Z[%d] : %02x\n", i, z)
	}
	println("Packet NUM =", h.Num)
}

type Packet struct {
	Tag uint16
	Ln  uint32
	D   []byte
}

func ReadPacket(r io.Reader) (p Packet, size int) {
	binary.Read(r, binary.LittleEndian, &p.Tag)
	binary.Read(r, binary.LittleEndian, &p.Ln)
	p.D = make([]byte, p.Ln)
	binary.Read(r, binary.LittleEndian, &p.D)

	return p, int(p.Ln + 6)
}

func (p *Packet) PrintHeader() {
	fmt.Printf("tag    : %d \n", p.Tag)
	fmt.Printf("length : %d \n", p.Ln)
}

func (p *Packet) PrintData(offset int) {
	o := offset + 2 + 4
	fmt.Printf("D[] offset: %08x(%d)\n", o, o)
	if p.Ln <= 16 {
		for i, d := range p.D {
			fmt.Printf("D[%d] : %02x = '%c'\n", i, d, d)
		}
	}
}

func main() {
	if len(os.Args) <= 1 {
		println("No args")
		return
	}
	println(os.Args[1])

	fp, err := os.Open(os.Args[1])
	defer fp.Close()
	if err != nil {
		println("File Open Error")
		return
	}

	fi, _ := fp.Stat()
	fileSize := int(fi.Size())
	println("File Size =", fileSize, "\n")

	offset := 0
	left := fileSize

	h, s := ReadHeader(fp)
	h.Print()
	offset += s
	left -= s
	fmt.Printf("left: %d\n", left)

	p := make([]Packet, h.Num)
	for i := uint32(0); i < h.Num; i++ {
		println("\nN =", i)
		p[i], s = ReadPacket(fp)
		p[i].PrintHeader()
		p[i].PrintData(offset)
		offset += s
		left -= s
		fmt.Printf("left: %d\n", left)
	}
}
