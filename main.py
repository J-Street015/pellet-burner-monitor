from flask import Flask, request
from models import PressureReading, Session


app = Flask(__name__)

# create and empty dictionary with the pressure key

data_dict = {"pressure": [0]}

@app.route("/")
def home():
    return("Flask is running")

@app.route("/data", methods=["POST"])
def receive_data():
    data = request.get_json()
    # print(data)
    data_dict['pressure'].append(data['pressure'])
    print(data_dict)
    reading = PressureReading(value_pa=data["pressure"])
    with Session() as session:
        session.add(reading)
        session.commit()


    return("OK") # returns ok back to the ESP32



if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5003)
