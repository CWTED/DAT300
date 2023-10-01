package change

import (
	"errors"
	"math"
	"sketch/sketch"
)

type Change struct {
	sk   *sketch.Sketch
	prev *sketch.Sketch
	erSk *sketch.Sketch
}

func FEDetect(s *sketch.Sketch, prev *sketch.Sketch, T uint64, key []byte) (uint64, uint64, error) {
	change := Change{sk: s, prev: prev}

	change.errorSketch()

	return reconstructFE(&change, key, T)
}

func (s *Change) errorSketch() {
	observed := sketch.ScalarSketch{Sketch: s.sk, Alpha: 1}
	previous := sketch.ScalarSketch{Sketch: s.prev, Alpha: -1}
	s.erSk = sketch.Combine(observed, previous)
}

// Choose threshold from c.skEr, T,
// reconstruct forecast error from given key and alert if above threshold TA
func reconstructFE(c *Change, key []byte, T uint64) (uint64, uint64, error) {

	TA := chooseThreshold(c, T)

	FE := c.erSk.Estimate(key)

	if FE > TA {
		return FE, TA, errors.New("Error over TA hreshold")
	} else {
		return FE, TA, nil
	}
}

// Choose threshold by altering T-value: Lower T gives lower threshold
func chooseThreshold(c *Change, T uint64) uint64 {

	return T * uint64(math.Pow(float64(c.erSk.EstimateF2()), float64(0.5)))

}
