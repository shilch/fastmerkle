package main

import (
	"bytes"
	"encoding/csv"
	"encoding/hex"
	"fmt"
	"io"
	"io/ioutil"
	"os"
)

// Converts blockchair transaction dump files to .bin block files
// https://gz.blockchair.com/bitcoin-sv/transactions/
func main(){
	if len(os.Args) < 3 {
		panic("Usage: dump2bin [dump file] [block height]")
	}

	path := os.Args[1]
	height := os.Args[2]

	file, err := os.OpenFile(path, os.O_RDONLY, 0)
	if err != nil {
		fmt.Fprintln(os.Stderr, "Failed to open file:", err)
		os.Exit(1)
	}
	defer file.Close()

	csvReader := csv.NewReader(file)
	csvReader.Comma = '\t'
	csvReader.FieldsPerRecord = -1

	buf := new(bytes.Buffer)

	for {
		line, err := csvReader.Read()
		if err != nil {
			if err == io.EOF {
				break
			}
			fmt.Fprintln(os.Stderr, "Failed to read dump:", err)
			os.Exit(1)
		}

		if len(line) < 2 || line[0] != height {
			continue
		}

		txId, err := hex.DecodeString(line[1])
		if err != nil {
			fmt.Fprintln(os.Stderr, "failed to decode transaction:", err)
			os.Exit(1)
		}

		for i := 31; i >= 0; i-- {
			err := buf.WriteByte(txId[i])
			if err != nil {
				fmt.Fprintln(os.Stderr, "failed to write transaction:", err)
				os.Exit(1)
			}
		}
	}

	err = ioutil.WriteFile(height + ".bin", buf.Bytes(), 0644)
	if err != nil {
		panic(err)
	}
}
