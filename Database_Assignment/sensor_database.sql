USE u515418176_AaronEstrada;

DROP TABLE IF EXISTS sensor_data;
DROP TABLE IF EXISTS sensor_register;

CREATE TABLE sensor_register (
    node_name VARCHAR(10) PRIMARY KEY,            
    manufacturer VARCHAR(10) NOT NULL,           
    longitude DECIMAL(15,8) NOT NULL,             
    latitude DECIMAL(15,8) NOT NULL               
);

CREATE TABLE sensor_data (
    node_name VARCHAR(10),                    
    time_received DATETIME NOT NULL,            
    temperature DECIMAL(6,2) CHECK (temperature BETWEEN -10 AND 100),
    humidity DECIMAL(6,2) CHECK (humidity BETWEEN 0 AND 100),
    FOREIGN KEY (node_name) REFERENCES sensor_register(node_name)
        ON DELETE CASCADE ON UPDATE CASCADE
);

INSERT INTO sensor_register (node_name, manufacturer, longitude, latitude) VALUES
('node_1', 'Aerovironment',     -122.70812, 38.34029),
('node_2', 'Cessna', -122.71087, 38.34195),
('node_3', 'Jump Aero', -122.71356, 38.34280),
('node_4', 'Northrop Grumman',  -122.71622, 38.34340),
('node_5', 'Lockheed Martin',  -122.71888, 38.34495);

INSERT INTO sensor_data (node_name, time_received, temperature, humidity) VALUES
('node_1', '2022-10-01 11:00:00', 22.5, 45.0),
('node_1', '2022-10-01 11:30:00', 23.1, 46.2),
('node_1', '2022-10-01 12:00:00', 23.9, 44.8),
('node_1', '2022-10-01 12:30:00', 24.3, 43.7),

('node_2', '2022-10-01 11:00:00', 25.4, 41.0),
('node_2', '2022-10-01 11:30:00', 26.0, 42.3),
('node_2', '2022-10-01 12:00:00', 27.2, 39.5),
('node_2', '2022-10-01 12:30:00', 27.8, 38.9),

('node_3', '2022-10-01 11:00:00', 18.2, 60.0),
('node_3', '2022-10-01 11:30:00', 18.5, 61.3),
('node_3', '2022-10-01 12:00:00', 19.0, 59.9),
('node_3', '2022-10-01 12:30:00', 19.5, 58.7),

('node_4', '2022-10-01 11:00:00', 29.0, 33.0),
('node_4', '2022-10-01 11:30:00', 29.8, 31.2),
('node_4', '2022-10-01 12:00:00', 30.5, 30.1),
('node_4', '2022-10-01 12:30:00', 31.0, 29.4),

('node_5', '2022-10-01 11:00:00', 20.3, 55.0),
('node_5', '2022-10-01 11:30:00', 21.0, 54.3),
('node_5', '2022-10-01 12:00:00', 21.7, 53.2),
('node_5', '2022-10-01 12:30:00', 22.1, 52.9);

SELECT * FROM sensor_register;
SELECT * FROM sensor_data;

CREATE OR REPLACE VIEW sensor_combined AS
SELECT 
    sr.node_name,
    sr.manufacturer,
    sr.longitude,
    sr.latitude,
    sd.time_received,
    sd.temperature,
    sd.humidity
FROM sensor_register sr
JOIN sensor_data sd ON sr.node_name = sd.node_name
ORDER BY sr.node_name, sd.time_received;

SELECT * FROM sensor_combined;
