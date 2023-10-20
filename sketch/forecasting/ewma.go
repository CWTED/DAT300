package forecasting

import (
	"sketch/sketch"
)

type EWMA struct {
	PreviousF *sketch.Sketch
	Alpha float64
}

func NewEWMA(alpha float64) *EWMA {
	return &EWMA{Alpha: alpha}
}

// Calculate the forecasted sketch using EWMA
func (ewma *EWMA) Forecast(prevO *sketch.Sketch) (*sketch.Sketch, error) {

	// If t = 2, then S_f(t) = S_o(1)
	if ewma.PreviousF == nil {
		ewma.PreviousF = prevO
		return prevO, nil
	} else {
		var observed = sketch.ScalarSketch{Sketch: prevO, Alpha: ewma.Alpha}
		var prevForecasted = sketch.ScalarSketch{Sketch: ewma.PreviousF, Alpha: 1 - ewma.Alpha}

		forecasted := sketch.Combine(observed, prevForecasted)
		ewma.PreviousF = forecasted
		return  forecasted, nil
	}
}

func (ewma *EWMA) Update(s *sketch.Sketch) {}