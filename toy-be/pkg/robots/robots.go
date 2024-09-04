package robots

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	"os"
	"time"
	"toy/errors"
	"toy/pkg/settings"

	posixMQ "github.com/syucream/posix_mq/src/posix_mq"
)

type RobotState struct {
	ID          uint64 `storm:"id" json:"id"`
	HostName    string `json:"hostname"`
	Temperature string `json:"temperature"`
	LMotorSpeed uint32 `json:"lmotor_speed"`
	RMotorSpeed uint32 `json:"rmotor_speed"`
	LMotorState uint32 `json:"lmotor_state"`
	RMotorState uint32 `json:"rmotor_state"`
}

const (
	RECV_MESSAGE_PATH = "/mq_sys_to_be"
	SEND_MESSAGE_PATH = "/mq_be_to_sys"
)

const MESSAGE_DATA_LEN = 50
const HOSTNAME_LEN = 32

const (
	MESSAGE_ID_EXIT = iota + 0
	MESSAGE_ID_INFO
	MESSAGE_ID_ENGINE
	MESSAGE_ID_MAX
)

type Sysmsg struct {
	Cmd  uint32
	Data [MESSAGE_DATA_LEN]byte
}

type RobotInfo struct {
	ID          uint32
	Hostname    [HOSTNAME_LEN]byte
	Temperature uint32
} // 40 byt

type MotorControl struct {
	id    uint32 // 0: Left, 1: Right
	speed uint32 // 0: halt, 1~100: speed
} //8 byte

var lateInit bool
var checkableFields = []string{
	"Name",
	"Name",
}

const (
	MonitoringDealy = time.Second * 5
)

func init() {
	lateInit = false
}

//nolint:gocyclo
func (p *RobotState) Clean(baseScope string, fields ...string) error {
	if len(fields) == 0 {
		fields = checkableFields
	}

	for _, field := range fields {
		switch field {
		case "ID":
			if p.HostName == "" {
				return errors.ErrEmptyRobotname
			}
		}
	}

	return nil
}

func checkError(err error, tmp string) {
	if err != nil {
		log.Println(tmp, ":", err)
	}
}

func SendEngineMessage(motor_id uint32, speed uint32) {
	var msg Sysmsg
	var motor MotorControl

	log.Println("send engine message")
	oflag := posixMQ.O_WRONLY
	sendQ, err := posixMQ.NewMessageQueue(SEND_MESSAGE_PATH, oflag, 0666, nil)

	if err != nil {
		log.Println(err)
		return
	}
	defer sendQ.Close()

	msg.Cmd = MESSAGE_ID_ENGINE
	motor.id = motor_id
	motor.speed = speed

	buf := &bytes.Buffer{}
	binary.Write(buf, binary.LittleEndian, &motor)
	copy(msg.Data[:], buf.Bytes())

	sbuf := &bytes.Buffer{}
	binary.Write(sbuf, binary.LittleEndian, msg)
	err = sendQ.Send(sbuf.Bytes(), 0)

	if err != nil {
		log.Println(err)
		return
	}
}

func RecvMessageQ(s Store, srv *settings.Server, hub *Hub) {
	var msg Sysmsg
	var robot RobotInfo
	var err error
	var mq *posixMQ.MessageQueue

	oflag := posixMQ.O_RDONLY
	mq, err = posixMQ.NewMessageQueue(RECV_MESSAGE_PATH, oflag, 0666, nil)
	for err != nil {
		if err != nil {
			log.Println("mq open error : ", err)
			time.Sleep(1 * time.Second)
		}
		mq, err = posixMQ.NewMessageQueue(RECV_MESSAGE_PATH, oflag, 0666, nil)
	}
	defer mq.Close()

	log.Println("Start receiving Loop")
	for {
		rcv, _, err := mq.Receive()
		checkErr(err, "mq.Receive")
		if err != nil {
			log.Printf("mq.Receive: error %s\n", err.Error())
		}

		buf := bytes.NewBuffer(rcv)
		if err := binary.Read(buf, binary.LittleEndian, &msg); err != nil {
			log.Println("binary.Read failed(msg):", err)
		}

		buf2 := bytes.NewBuffer(msg.Data[:])
		switch msg.Cmd {
		case MESSAGE_ID_INFO:
			if err := binary.Read(buf2, binary.LittleEndian, &robot); err != nil {
				log.Println("binary.Read failed(JobData):", err)
			} else {
				log.Println("Robot info")
				UpdateRobot(hub, s, srv.Root, uint64(robot.ID), string(robot.Hostname[0:32]), fmt.Sprint(robot.Temperature))
			}
		default:
			log.Println("unknown message.")
		}
	}
}

func UpdateRobot(hub *Hub, s Store, root string, id uint64, hostname string, temperature string) {
	// DB에서 값을 읽고
	p, err := s.Get(root, id)
	checkErr(err, "UpdateRobot")

	p.HostName, _ = os.Hostname()
	p.Temperature = temperature

	// DB 값을 수정
	err = s.Update(p)
	checkErr(err, "UpdateRobot")

	BroadcastRobot(hub, id, p.HostName, temperature)
}

func RobotThread(s Store, srv *settings.Server, hub *Hub) {

	go RecvMessageQ(s, srv, hub)
	for {
		time.Sleep(time.Second * 3)
		// SendEngineMessage(0, 50)
		// time.Sleep(time.Second * 1)
		// SendEngineMessage(0, 80)
		// time.Sleep(time.Second * 3)
		// SendEngineMessage(1, 10)
		// time.Sleep(time.Second * 3)
		// SendEngineMessage(1, 30)
		// time.Sleep(time.Second * 3)
		// SendEngineMessage(0, 0)
		// time.Sleep(time.Second * 3)
		// SendEngineMessage(1, 0)
	}
}
