package forecasting

import (
	"sketch/sketch"
)

type Forecasting interface {
	Forecast(prevO *sketch.Sketch) (*sketch.Sketch, error)
	Update(s *sketch.Sketch)
}