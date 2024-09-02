package http

import (
	"encoding/json"
	"net/http"
	"sort"
	"strconv"

	"toy/errors"
	"toy/pkg/robots"
)

type modifyRobotRequest struct {
	modifyRequest
	Data *robots.RobotState `json:"data"`
}

type Robots struct {
	Count    int                  `json:"count"`
	Next     string               `json:"next"`
	Previous string               `json:"previous"`
	Results  []*robots.RobotState `json:"results"`
}

func getRobot(_ http.ResponseWriter, r *http.Request) (*modifyRobotRequest, error) {
	if r.Body == nil {
		return nil, errors.ErrEmptyRequest
	}

	req := &modifyRobotRequest{}
	err := json.NewDecoder(r.Body).Decode(req)
	if err != nil {
		return nil, err
	}

	if req.What != "robot" {
		return nil, errors.ErrInvalidDataType
	}

	return req, nil
}

var robotsGetHandler = withUser(func(w http.ResponseWriter, r *http.Request, d *data) (int, error) {
	robots, err := d.store.Robots.Gets(d.server.Root)
	if err != nil {
		return http.StatusInternalServerError, err
	}

	sort.Slice(robots, func(i, j int) bool {
		return robots[i].ID < robots[j].ID
	})

	pdata := make([]Robots, 1)
	pdata[0].Count = len(robots)
	pdata[0].Results = robots

	return renderJSON(w, r, pdata[0])
})

var robotGetByIDHandler = withUser(func(w http.ResponseWriter, r *http.Request, d *data) (int, error) {
	u, err := d.store.Robots.Get(d.server.Root, d.raw.(uint64))
	if err == errors.ErrNotExist {
		return http.StatusNotFound, err
	}

	if err != nil {
		return http.StatusInternalServerError, err
	}

	return renderJSON(w, r, u)
})

var robotDeleteHandler = withUser(func(w http.ResponseWriter, r *http.Request, d *data) (int, error) {
	err := d.store.Robots.Delete(d.raw.(uint64))
	if err != nil {
		return errToStatus(err), err
	}

	return http.StatusOK, nil
})

var robotsPostHandler = withUser(func(w http.ResponseWriter, r *http.Request, d *data) (int, error) {
	req, err := getRobot(w, r)
	if err != nil {
		return http.StatusBadRequest, err
	}

	if len(req.Which) != 0 {
		return http.StatusBadRequest, nil
	}

	err = d.store.Robots.Save(req.Data)
	if err != nil {
		return http.StatusInternalServerError, err
	}

	w.Header().Set("Location", "/settings/robots/"+strconv.FormatUint(uint64(req.Data.ID), 10))
	return http.StatusCreated, nil
})

var robotPutByIDHandler = withUser(func(w http.ResponseWriter, r *http.Request, d *data) (int, error) {
	req, err := getRobot(w, r)
	if err != nil {
		return http.StatusBadRequest, err
	}

	if len(req.Which) == 0 || (len(req.Which) == 1 && req.Which[0] == "all") {
		err = d.store.Robots.Update(req.Data)
		if err != nil {
			return http.StatusInternalServerError, err
		}

		return http.StatusOK, nil
	}

	err = d.store.Robots.Update(req.Data, req.Which...)
	if err != nil {
		return http.StatusInternalServerError, err
	}

	return http.StatusOK, nil
})
