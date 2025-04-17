package main

import (
	"fmt"
	"os"
)

func main() {
	if len(os.Args) < 3 {
		fmt.Fprintf(os.Stderr, "No args\n")
		os.Exit(1)
	}

	orig, err := os.ReadFile(os.Args[1])
	if err != nil {
		panic(err)
	}

	reverse := make([]byte, len(orig))
	for i, o := range orig {
		r := byte(0)
		for b := 7; b >= 0; b-- {
			r |= (o & 1) << b
			o >>= 1
		}
		reverse[i] = r
	}

	err = os.WriteFile(os.Args[2], reverse, 0666)
	if err != nil {
		panic(err)
	}
}
