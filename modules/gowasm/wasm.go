package main

//go:wasm-module arduino
//go:export runner
func runner(js int) int {
	return js + 1
}

func main() {
	for {
	}
}
