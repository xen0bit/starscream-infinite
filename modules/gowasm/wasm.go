package main

import (
	"math"
)

func main() {
	<-make(chan bool)
}

//export runner
func runner(js int) int {
	if js < 2 {
		//fmt.Println("Number must be greater than 2.")
		return 0
	}
	sq_root := int(math.Sqrt(float64(js)))
	for i := 2; i <= sq_root; i++ {
		if js%i == 0 {
			//fmt.Println("Non Prime Number")
			return 0
		}
	}
	//fmt.Println("Prime Number")
	return js
}
