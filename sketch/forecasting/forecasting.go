package forecasting

import (
	"sketch"
)

type Forecasting interface {
	Forecast(prevO *sketch.Sketch) (sketch.Sketch, error)
}