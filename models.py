# here the SQl Alchemy database is created.
# imports
from datetime import datetime, timezone
from zoneinfo import ZoneInfo
from sqlalchemy import create_engine, Integer, Float, DateTime
from sqlalchemy.orm import DeclarativeBase, Mapped, mapped_column, sessionmaker

'''SQLAlchemy is split into two parts:
sqlalchemy (top level) gives you the connection tools (create_engine) and the column types (Integer, Float, DateTime).
sqlalchemy.orm gives you the object side: base class, column declarations, and sessions.'''

'''Step 1: the declarative base
The declarative base is a registry. 
It keeps track of every class that describes a table, 
so that when we ask SQLAlchemy to create the tables, 
it knows which classes to turn into tables. Think of it as the list of table blueprints for the project.
'''


class Base(DeclarativeBase):
    pass

'''DeclarativeBase is the SQLAlchemy class we inherit from. pass means the class adds nothing of its own. 
Its only job is to be the parent that every table class inherits from later.'''


'''Step 2: the engine
The engine is the connection to the database file. It knows which database type to use (SQLite), 
where the file is (pressure.db), and how to talk to it. Creating the engine doesn’t write anything yet.

The sqlite:/// prefix means “SQLite file”, and the three slashes mean 
the path is relative to the folder you run the script from.
'''
engine = create_engine("sqlite:///heatsystem.db")


'''Step 3: the session factory
A session is a workspace where you collect changes before writing them. 
The session factory is a small builder that produces new sessions connected to our engine.
Calling Session() later gives you a fresh session. Nothing reaches the file until you call commit() on it.

'''
Session = sessionmaker(bind=engine)

'''Step 4: the table class
This is where the table’s structure is defined. Each class that inherits from Base becomes one table, 
and each attribute becomes one column.
Inheriting from Base registers the class. __tablename__ sets the name in the file. 
Then each column is defined: id numbers the rows automatically, 
timestamp records the arrival time in UTC, and value_pa holds the pressure you send in.

the time stamp could also be set like this in the table 
timestamp: Mapped[datetime] = mapped_column(DateTime)
and then in the flask endpoint where the data is added the time would be added like this. 
reading = PressureReading(
    value_pa=data["pressure"],
    timestamp=datetime.now(timezone.utc),
    )
'''

class PressureReading(Base):
    __tablename__ = "pressure_readings"

    id: Mapped[int] = mapped_column(Integer, primary_key=True)
    timestamp: Mapped[datetime] = mapped_column(DateTime, default=lambda: datetime.now(tz=ZoneInfo("Europe/Stockholm")).replace(microsecond=0))
    value_pa: Mapped[float] = mapped_column(Float)


'''Step 5: create the tables
The last step applies all the blueprints to the file.
Base.metadata is the list of every table registered on Base. 
create_all creates any table that doesn’t exist yet and leaves existing tables alone, 
so running the script again doesn’t erase your data.

The lambda function in this context 
default= expects a function, not a value. SQLAlchemy calls that function each time it creates a new row. 
So the sequence is:
You send a reading.
SQLAlchemy creates a new PressureReading.
It calls the function you gave in default=, which returns the current time.
That time is stored in timestamp.

IMPORTANT!!!
Why not just write the time directly
If you wrote default=datetime.now(timezone.utc) without the lambda, Python would run datetime.now() once, 
when the script loads, and use that fixed value forever. 
Every row would get the same timestamp: the time the script started. 
The lambda delays the call until a row is actually created.

'''
Base.metadata.create_all(engine)



