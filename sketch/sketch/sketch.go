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
    h    uint
    k    uint
    count [][]uint64
    hasher hash.Hash64
}

// Creates a new sketch struture
// Imported from https://github.com/shenwei356/countminsketch/blob/master/countminsketch.go
func New(h uint, k uint) (s *Sketch, err error) {
	if h <= 0 || k <= 0 {
		return nil, errors.New("countminsketch: values of d and w should both be greater than 0")
	}

	s = &Sketch{
		h:      h,
		k:      k,
		hasher: fnv.New64(),
	}
	s.count = make([][]uint64, h)
	for r := uint(0); r < h; r++ {
		s.count[r] = make([]uint64, k)
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

	k := uint(math.Ceil(2 / epsilon))
	h := uint(math.Ceil(math.Log(1-delta) / math.Log(0.5)))
	// fmt.Printf("ε: %f, δ: %f -> d: %d, w: %d\n", epsilon, delta, d, w)
	return New(h, k)
}


// Update the frequency of a key
// Imported from https://github.com/shenwei356/countminsketch/blob/master/countminsketch.go
func (s *Sketch) Update(key []byte, count uint64) {
	for r, c := range s.locations(key) {
		s.count[r][c] += count
	}
}

// Estimate a given key
func (s *Sketch) Estimate(key []byte) uint64 {
    var vEst uint64


    var v = make([]uint64, s.H())

    for index, val := range s.locations(key) {
        v[index] = (s.count[index][val] - s.sum()/(uint64(s.K()))) / uint64(1 - 1 / s.K())
    }

    vEst = median(v)

    return vEst
}

// Estimate the second moment 
func (s *Sketch) EstimateF2() uint64 {
    var f2Est uint64

    var f2 = make([]uint64, s.H())

    for index, row := range s.count {
        var temp uint64

        for _, val := range row {
            temp += val^2
        }

        f2[index] = uint64(s.K() / (s.K() - 1)) * temp - uint64(1 / (s.K() - 1)) * s.sum()^2
    }

    f2Est = median(f2)

    return f2Est
}

// Combine multiple sketches
func Combine(elem ... ScalarSketch) *Sketch {
    combinedSketch, _ := New(elem[0].Sketch.h, elem[0].Sketch.k)

    for i, row := range elem[0].Sketch.count {
        for j := range row {
            var sum float64
            for _, scalarSketch := range elem {
                sum += float64(scalarSketch.Sketch.count[i][j]) * scalarSketch.Alpha
            }

            combinedSketch.count[i][j] = uint64(sum)
        }
    }

    return combinedSketch
}


func (s *Sketch) Print() {
    for _, row := range s.count {
        for _, val := range row {
            fmt.Printf("%d ", val)
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
func (s *Sketch) H() uint {
	return s.h
}

// W returns the size of hashing functions
func (s *Sketch) K() uint {
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
	for r := uint(0); r < s.h; r++ {
		locs[r] = (ua + ub*r) % s.k
	}
	return
}

func median(arr []uint64) uint64 {
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

func (s *Sketch) sum() uint64 {
    var sum uint64
    for _, val := range s.count[0] {
        sum += val
    }

    return sum
}