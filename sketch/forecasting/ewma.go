package forecasting

import (
	"errors"
	"sketch/sketch"
)

type EWMA struct {
	previousF *sketch.Sketch
	alpha float64
}

// Calculate the forecasted sketch using EWMA
func (ewma *EWMA) Forecast(prevO *sketch.Sketch) (*sketch.Sketch, error) {
	if prevO == nil {
		return nil, errors.New("EWMA: previous observed sketch is nil")
	}

	// If t = 2, then S_f(t) = S_o(1)
	if ewma.previousF == nil {
		ewma.previousF = prevO
		return prevO, nil
	} else {
		var observed = sketch.ScalarSketch{Sketch: prevO, Alpha: ewma.alpha}
		var prevForecasted = sketch.ScalarSketch{Sketch: ewma.previousF, Alpha: 1 - ewma.alpha}

		forecasted := sketch.Combine(observed, prevForecasted)
		ewma.previousF = forecasted
		return  forecasted, nil
	}
}