package main

import (
	"fmt"
	"net/http"
	"strconv"

	"github.com/gin-gonic/gin"
)

var (
	clientId = 0
	jobsch   = make(chan int)
	clients  = make(map[string]int)
)

func register(c *gin.Context) {
	clientId += 1
	clients[strconv.Itoa(clientId)] = 0
	c.String(http.StatusOK, strconv.Itoa(clientId))
}

func job(c *gin.Context) {
	id := c.Param("id")
	clients[id] = clients[id] + 1
	c.String(http.StatusOK, strconv.Itoa(<-jobsch))
}

func solve(c *gin.Context) {
	solution := c.Param("solution")
	fmt.Println("Solution: " + solution)
}

func jobGenerator(jc chan int) {
	for i := 0; i < 1000000; i++ {
		jc <- i
	}
	close(jc)
}

func main() {
	go jobGenerator(jobsch)
	router := gin.Default()
	router.GET("/register", register)
	router.StaticFile("/module", "./runner.wasm")
	router.GET("/job/:id", job)
	router.GET("/solve/:solution", solve)
	router.Run("localhost:8080")
}
