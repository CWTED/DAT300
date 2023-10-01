package change

import (
	"errors"
	"math"
	"sketch/sketch"
)

type Change struct {
	obs   *sketch.Sketch // Observed sketch
	forec *sketch.Sketch // Forecasted sketcj
	erSk  *sketch.Sketch // Error sketch
}

func FEDetect(observed *sketch.Sketch, forecasted *sketch.Sketch, T float64, key []byte) (float64, float64, error) {
	change := Change{obs: observed, forec: forecasted}
	change.errorSketch()

	return reconstructFE(&change, key, T)
}

func (s *Change) errorSketch() {
	observed := sketch.ScalarSketch{Sketch: s.obs, Alpha: 1}
	previous := sketch.ScalarSketch{Sketch: s.forec, Alpha: -1}
	s.erSk = sketch.Combine(observed, previous)
}

// Choose threshold from c.skEr, T,
// reconstruct forecast error from given key and alert if above threshold TA
func reconstructFE(c *Change, key []byte, T float64) (float64, float64, error) {

	TA := chooseThreshold(c, T)

	FE := c.erSk.Estimate(key)
	//fmt.Printf("%f\t%f\n", TA, FE)
	if FE > TA {
		return FE, TA, errors.New("Error over TA hreshold")
	} else {
		return FE, TA, nil
	}
}

// Choose threshold by altering T-value: Lower T gives lower threshold
func chooseThreshold(c *Change, T float64) float64 {
	return (T * math.Pow(float64(c.erSk.EstimateF2()), float64(0.5)))
}
