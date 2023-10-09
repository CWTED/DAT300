package forecasting

import (
	"sketch/sketch"
)

type AccVel struct {
	previousForecast *sketch.Sketch
	Window int
	vSketches []*sketch.Sketch
	previousVelocity *sketch.Sketch
}

func (accvel *AccVel) Forecast(prevO *sketch.Sketch) (*sketch.Sketch, error) {
	if accvel.previousVelocity == nil {
		accvel.previousVelocity, _ = sketch.New(prevO.H(), prevO.K())
	}
	if accvel.previousForecast == nil {
		accvel.previousForecast = prevO
	}
	velocity := sketch.Combine(sketch.ScalarSketch{Sketch: accvel.vSketches[len(accvel.vSketches) - 1], Alpha: 1.0/float64(accvel.Window)}, 
							   sketch.ScalarSketch{Sketch: accvel.vSketches[0], Alpha: -1.0/float64(accvel.Window)})



	acceleration := sketch.Combine(sketch.ScalarSketch{Sketch: velocity, Alpha: 1}, sketch.ScalarSketch{Sketch: accvel.previousVelocity, Alpha: -1})

	accvel.previousForecast = sketch.Combine(sketch.ScalarSketch{Sketch: accvel.previousForecast, Alpha: 1},
											  sketch.ScalarSketch{Sketch: velocity, Alpha: 1},
											  sketch.ScalarSketch{Sketch: acceleration, Alpha: 1})
	
	return accvel.previousForecast, nil
}

func (accvel *AccVel) UpdateVelocity(s *sketch.Sketch) {
	accvel.vSketches = append(accvel.vSketches, s)	

	if len(accvel.vSketches) > accvel.Window {
		accvel.vSketches = accvel.vSketches[1:]
	}
}