package sketch

import (
    "fmt"
    "hash"
    "errors"
    "hash/fnv"
    "math"
    "encoding/binary"
    "sort"
)

type Sketch struct {
    h    int
    k    int
    Count [][]float64
    hasher hash.Hash64
}

// Creates a new sketch struture
// Imported from https://github.com/shenwei356/countminsketch/blob/master/countminsketch.go
func New(h int, k int) (s *Sketch, err error) {
	if h <= 0 || k <= 0 {
		return nil, errors.New("countminsketch: values of d and w should both be greater than 0")
	}

	s = &Sketch{
		h:      h,
		k:      k,
		hasher: fnv.New64(),
	}
	s.Count = make([][]float64, h)
	for r := uint(0); r < uint(h); r++ {
		s.Count[r] = make([]float64, k)
	}

	return s, nil
}

// Creates new sketch with given error parameters
// Imported from https://github.com/shenwei356/countminsketch/blob/master/countminsketch.go
func NewWithEstimates(epsilon, delta float64) (*Sketch, error) {
	if epsilon <= 0 || epsilon >= 1 {
		return nil, errors.New("countminsketch: value of epsilon should be in range of (0, 1)")
	}
	if delta <= 0 || delta >= 1 {
		return nil, errors.New("countminsketch: value of delta should be in range of (0, 1)")
	}

	k := int(math.Ceil(2 / epsilon))
	h := int(math.Ceil(math.Log(1-delta) / math.Log(0.5)))
	// fmt.Printf("ε: %f, δ: %f -> d: %d, w: %d\n", epsilon, delta, d, w)
	return New(h, k)
}


// Update the frequency of a key
// Imported from https://github.com/shenwei356/countminsketch/blob/master/countminsketch.go
func (s *Sketch) Update(key []byte, count float64) {
	for r, c := range s.locations(key) {
		s.Count[r][c] += count
	}
}

// Estimate a given key
func (s *Sketch) Estimate(key []byte) float64 {
    var vEst float64


    var v = make([]float64, s.H())

    for index, val := range s.locations(key) {
        v[index] = (s.Count[index][val] - s.sum()/float64(s.k)) / (1 - 1 / float64(s.K()))
    }

    vEst = median(v)

    return vEst
}

// Estimate the second moment 
func (s *Sketch) EstimateF2() float64 {
    var f2Est float64

    var f2 = make([]float64, s.H())

    for index, row := range s.Count {
        var temp float64

        for _, val := range row {
            temp += val * val
        }

        f2[index] = (float64(s.K()) / float64(s.K() - 1)) * temp - (1 / (float64(s.K()) - 1)) * (s.sum() * s.sum())
    }

    f2Est = median(f2)

    return f2Est
}

// Combine multiple sketches
func Combine(elem ... ScalarSketch) *Sketch {
    combinedSketch, _ := New(elem[0].Sketch.h, elem[0].Sketch.k)

    for i, row := range elem[0].Sketch.Count {
        for j := range row {
            var sum float64
            for _, scalarSketch := range elem {
                sum += float64(scalarSketch.Sketch.Count[i][j]) * scalarSketch.Alpha
            }

            combinedSketch.Count[i][j] = sum
        }
    }

    return combinedSketch
}


func (s *Sketch) Print() {
    for _, row := range s.Count {
        for _, val := range row {
            fmt.Printf("%f ", val)
        }
        fmt.Println()
    }
}

// ------------------------------------ HELPER FUNCTIONS ---------------------------------------------------
type ScalarSketch struct {
    Sketch *Sketch
    Alpha float64
}
// D returns the number of hashing functions
func (s *Sketch) H() int {
	return s.h
}

// W returns the size of hashing functions
func (s *Sketch) K() int {
	return s.k
}


// get the two basic hash function values for data.
// Based on https://github.com/willf/bloom/blob/master/bloom.go
func (s *Sketch) baseHashes(key []byte) (a uint32, b uint32) {
	s.hasher.Reset()
	s.hasher.Write(key)
	sum := s.hasher.Sum(nil)
	upper := sum[0:4]
	lower := sum[4:8]
	a = binary.BigEndian.Uint32(lower)
	b = binary.BigEndian.Uint32(upper)
	return
}

// Get the _w_ locations to update/Estimate
// Based on https://github.com/willf/bloom/blob/master/bloom.go
func (s *Sketch) locations(key []byte) (locs []uint) {
	locs = make([]uint, s.h)
	a, b := s.baseHashes(key)
	ua := uint(a)
	ub := uint(b)
	for r := uint(0); r < uint(s.h); r++ {
		locs[r] = (ua + ub*r) % uint(s.k)
	}
	return
}

func median(arr []float64) float64 {
    sort.Slice(arr, func(i, j int) bool {return arr[i] < arr[j]})   // Adapted from https://stackoverflow.com/questions/38607733/sorting-a-uint64-slice-in-go

    len := len(arr)

    if len == 0 {
        return 0
    } else if len % 2 == 0 {
        return (arr[len/2 -1] + arr[len/2]) / 2
    } else {
        return arr[len/2]
    }
}

func (s *Sketch) sum() float64 {
    var sum float64
    for _, val := range s.Count[0] {
        sum += val
    }

    return sum
}