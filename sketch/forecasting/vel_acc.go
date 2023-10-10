package forecasting

import (
	"sketch/sketch"
	"fmt"
)

type AccVel struct {
	previousForecast *sketch.Sketch
	Window int
	vSketches []*sketch.Sketch
	previousVelocity *sketch.Sketch
	index int
}

func New(window int, h int, k int) *AccVel {
	vS := make([]*sketch.Sketch, window)
	for index := range vS {
		vS[index], _ = sketch.New(h, k)
	}
	
	return &AccVel{Window: window, vSketches: vS, index: 0}
}

func (accvel *AccVel) Forecast(prevO *sketch.Sketch) (*sketch.Sketch, error) {
	if accvel.previousVelocity == nil {
		accvel.previousVelocity, _ = sketch.New(prevO.H(), prevO.K())
	}
	if accvel.previousForecast == nil {
		accvel.previousForecast = prevO
	}
	velocity := sketch.Combine(sketch.ScalarSketch{Sketch: accvel.vSketches[len(accvel.vSketches) - 1], Alpha: 1}, 
							   sketch.ScalarSketch{Sketch: accvel.vSketches[0], Alpha: -1})
	fmt.Println("-----------")
	velocity.Print()


	acceleration := sketch.Combine(sketch.ScalarSketch{Sketch: velocity, Alpha: 1}, sketch.ScalarSketch{Sketch: accvel.previousVelocity, Alpha: -1})

	accvel.previousForecast = sketch.Combine(sketch.ScalarSketch{Sketch: accvel.previousForecast, Alpha: 1},
											  sketch.ScalarSketch{Sketch: velocity, Alpha: 1},
											  sketch.ScalarSketch{Sketch: acceleration, Alpha: 1})
	
	return accvel.previousForecast, nil
}

func (accvel *AccVel) UpdateVelocity(s *sketch.Sketch) {
	if accvel.index < accvel.Window && accvel.index != 0 {
		prev := s.Count
		for i := accvel.index; i >= 0; i-- {
			temp := accvel.vSketches[i].Count
			accvel.vSketches[i].Count = prev
			prev = temp
		}
	} else if accvel.index >= accvel.Window {
		prev := s.Count
		for i := accvel.Window - 1; i >= 0; i-- {
			temp := accvel.vSketches[i].Count
			accvel.vSketches[i].Count = prev
			prev = temp
		}

	} else {
		accvel.vSketches[0].Count = s.Count
	}
	fmt.Println("-----------")
	accvel.vSketches[0].Print()
	fmt.Println()
	accvel.vSketches[len(accvel.vSketches) - 1].Print()
	accvel.index++
}