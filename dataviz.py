# this script uses the database created in flask from ESP32 sensor readings
# displays the table with the sensor readings as a table and also as a line chart.

import streamlit as st
import pandas as pd


# read in the data


# define the functions to either show the table or the plotted data
@st.fragment(run_every=2)
def show_table():
    df = pd.read_sql("SELECT timestamp, value_pa FROM pressure_readings", "sqlite:///heatsystem.db")
    # import the database with pandas
    df['timestamp'] = pd.to_datetime(df.timestamp)
    st.dataframe(df)


@st.fragment(run_every=2)
def show_plot():
    df = pd.read_sql("SELECT timestamp, value_pa FROM pressure_readings", "sqlite:///heatsystem.db")
    # import the database with pandas
    df['timestamp'] = pd.to_datetime(df.timestamp)
    df = df.set_index("timestamp")
    df = df.resample("5min").mean()
    st.line_chart(df, x_label="Time", y_label="Pressure in Pa", color='green')
    st.write(df.describe())

with st.sidebar:
    selection = st.selectbox("Menu", options=("Overview", "Pressure Table", "Plot"))

if selection == "Overview":
    # define container for the data
    with st.container(border=True):
        st.title("Pressure data Overview")

        #inside the container define columns
        col1, col2 = st.columns(2)

        # defines the left column
        with col1:
            st.write("Data Table with measured pressure")
            show_table()

        # defines the right column
        with col2:
            st.write("Data from Table plotted over time. \n Data is presented as the mean over 5 minutes.")
            show_plot()

if selection == "Pressure Table":
    st.title("Pressure Readings from Sensor")
    st.write("Measurement in a table format.")
    show_table()

if selection == "Plot":
    st.title("Pressure Readings from Sensor")
    st.write("Measurements in a line plot with resampled values with 5min mean values.")
    show_plot()

# run the script from Terminal using: streamlit run dataviz.py --server.address 0.0.0.0
