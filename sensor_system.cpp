#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/select.h>
#include <ctime>

using namespace std;

/* -------- sysfs helpers -------- */

int readInt(const string &path)  
{
    ifstream file(path);
    int value = 0;
    file >> value;
    return value;
}

void writeValue(const string &path, int value)
{
    ofstream file(path);
    file << value;
}

/* -------- sensor paths -------- */

const string TEMP_PATH =
"/sys/bus/iio/devices/iio:device0/in_temp_input";

const string HUM_PATH =
"/sys/bus/iio/devices/iio:device0/in_humidityrelative_input";

/* LEDs */
const string LED_GREEN =
"/sys/class/leds/gpio20_led/brightness";

const string LED_RED =
"/sys/class/leds/gpio21_led/brightness";

/* buzzer */
const string BUZZER =
"/sys/class/leds/gpio_buzzer/brightness";

/* input device */

const char *INPUT_DEV = "/dev/input/event0";

/* flags */

bool gasDetected = false;
bool flameDetected = false;
bool buttonPressed = false;

/* confirmation counters */

int gasCounter = 0;
int flameCounter = 0;
int tempCounter = 0;

const int CONFIRM_LIMIT = 3;

/* -------- read events -------- */

void checkEvents(int fd)
{
    struct input_event ev;

    while(read(fd, &ev, sizeof(ev)) > 0)
    {
        if(ev.type == EV_KEY && ev.value == 1)
        {
            if(ev.code == KEY_PROG1)
                gasDetected = true;

            if(ev.code == KEY_PROG2)
                flameDetected = true;

            if(ev.code == KEY_ENTER)
                buttonPressed = true;
        }
    }
}

/* -------- timestamp -------- */

string getTimestamp()
{
    time_t now = time(0);
    char buf[64];

    strftime(buf,sizeof(buf), "%Y-%m-%d %H:%M:%S",localtime(&now));

    return string(buf);
}

/* -------- send to server -------- */

void sendToServer(const string &json)
{
    string cmd =
    "curl -s -X POST http://127.0.0.1:5000/sensor-data "
    "-H \"Content-Type: application/json\" "
    "-H \"X-API-KEY: 12345\" "
    "-d '" + json + "'";

    system(cmd.c_str());
}

/* -------- main -------- */

int main()
{
    int fd = open(INPUT_DEV,O_RDONLY | O_NONBLOCK);

    if(fd < 0)
    {
        cout << "Cannot open input device\n";
        return 1;
    }

    while(true)
    {

        /* read sensors */

        float temperature = readInt(TEMP_PATH) / 1000.0;

        float humidity = readInt(HUM_PATH) / 1000.0; 

        /* ignore bad DHT readings */

        if(temperature <= 0 || humidity <= 0 || humidity > 100 || temperature > 80)
        {
            //cout << "DHT sensor error... skipping send\n";
            sleep(3);
            continue;
        }

        /* events */

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd,&fds);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        if(select(fd+1,&fds,NULL,NULL,&tv) > 0)
        {
            checkEvents(fd);
        }

        bool gas = gasDetected;
        bool flame = flameDetected;
        bool highTemp = (temperature > 60);

        if(gas) gasCounter++;
        else gasCounter = 0;

        if(flame) flameCounter++;
        else flameCounter = 0;

        if(highTemp) tempCounter++;
        else tempCounter = 0;

        bool gasAlarm = gasCounter >= CONFIRM_LIMIT;

        bool flameAlarm = flameCounter >= CONFIRM_LIMIT;

        bool tempAlarm = tempCounter >= CONFIRM_LIMIT;

        bool alarm = gasAlarm || flameAlarm || tempAlarm;

        /* system */

        if(alarm)
        {
            writeValue(LED_GREEN,0);
            writeValue(LED_RED,1);
            writeValue(BUZZER,1);
        }
        else
        {
            writeValue(LED_GREEN,1);
            writeValue(LED_RED,0);
            writeValue(BUZZER,0);
        }

        /* reset button */  

        if(buttonPressed)
        {
            gasCounter = 0;
            flameCounter = 0;
            tempCounter = 0;

            writeValue(LED_RED,0);
            writeValue(BUZZER,0);
            writeValue(LED_GREEN,1);

            cout << "SYSTEM RESET\n";

            buttonPressed = false;
        }

        /* JSON */ 

        stringstream json; 

        json << "{";
        json << "\"temperature\":" << temperature << ",";
        json << "\"humidity\":" << humidity << ",";
        json << "\"gas\":" << (gasAlarm ? 1 : 0) << ",";
        json << "\"flame\":" << (flameAlarm ? 1 : 0) << ",";
        json << "\"alarm\":" << (alarm ? 1 : 0) << ",";
        json << "\"timestamp\":\"" << getTimestamp() << "\"";
        json << "}";

        cout << json.str() << endl;

        sendToServer(json.str());

        gasDetected = false;
        flameDetected = false;

        sleep(3);
    }

    close(fd);
}