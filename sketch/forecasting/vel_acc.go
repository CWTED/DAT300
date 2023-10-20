package forecasting

import (
	"sketch/sketch"
)

type AccVel struct {
	Window int
	vSketches []*sketch.Sketch
	previousVelocity *sketch.Sketch
	index int
}

func NewVelAcc(window int, h int, k int) *AccVel {
	vS := make([]*sketch.Sketch, window)
	for index := range vS {
		vS[index], _ = sketch.New(h, k)
	}
	previousVelocity, _ := sketch.New(h, k)

	return &AccVel{Window: window, vSketches: vS, index: 0, previousVelocity: previousVelocity}
}

func (accvel *AccVel) Forecast(prevO *sketch.Sketch) (*sketch.Sketch, error) {
	// Calculate velocity element; difference between the latest and earliest sketch within the sliding window
	velocity := sketch.Combine(sketch.ScalarSketch{Sketch: accvel.vSketches[len(accvel.vSketches) - 1], Alpha: 1}, 
							   sketch.ScalarSketch{Sketch: accvel.vSketches[0], Alpha: -1})

	// Acceleration is the difference between the current and previous velocity
	acceleration := sketch.Combine(sketch.ScalarSketch{Sketch: velocity, Alpha: 1}, sketch.ScalarSketch{Sketch: accvel.previousVelocity, Alpha: -1})

	// New forcast is the sum of the observed sketch, velocity and acceleration
	forecast := sketch.Combine(sketch.ScalarSketch{Sketch: prevO, Alpha: 1},
											  sketch.ScalarSketch{Sketch: velocity, Alpha: 1},
											  sketch.ScalarSketch{Sketch: acceleration, Alpha: 1})
	
	accvel.previousVelocity.Count = copyCount(velocity.Count)
	return forecast, nil
}

// Updates the sliding window with new the latest observed sketch
func (accvel *AccVel) Update(s *sketch.Sketch) {
	var prev [][]float64
	if accvel.index < accvel.Window && accvel.index != 0 {
		prev = copyCount(s.Count)
		for i := accvel.index; i >= 0; i-- {
			temp := copyCount(accvel.vSketches[i].Count)
			accvel.vSketches[i].Count = copyCount(prev)
			prev = temp
		}
	} else if accvel.index >= accvel.Window {
		prev = copyCount(s.Count)
		for i := accvel.Window - 1; i >= 0; i-- {
			temp := copyCount(accvel.vSketches[i].Count)
			accvel.vSketches[i].Count = copyCount(prev)
			prev = temp
		}

	} else {
		accvel.vSketches[0].Count = copyCount(s.Count)
	}
	accvel.index++
}

// Helper function. Copy the count in sketch structure
func copyCount(src [][]float64) [][]float64 {
	dst := make([][]float64, len(src))
	for i := range src {
		dst[i] = make([]float64, len(src[i]))
		copy(dst[i], src[i])
	}
	return dst
}