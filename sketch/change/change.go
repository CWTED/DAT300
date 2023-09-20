package change

import (
	"sketch/sketch"
	"math"
	"errors"
)



// Choose threshold by altering T-value: Lower T gives lower threshold
func chooseThreshold(s *sketch.Sketch, T uint64) uint64 {

	return T * uint64(math.Pow(float64(s.EstimateF2()),float64(0.5)))
	
}

// Choose threshold from s, T, 
// reconstruct forecast error from given key and alert if above threshold TA
func reconstructFE(s *sketch.Sketch, key []byte, T uint64) (uint64, uint64, error) {

	TA := chooseThreshold(s,T)

	FE := s.Estimate(key)

	if(FE > TA) {
		return FE, TA, errors.New("Error over TA hreshold")
	} else {
		return FE, TA, nil
	}

}
