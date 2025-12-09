CREATE DATABASE IF NOT EXISTS `web_temp_db` 
CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE `web_temp_db`;


CREATE TABLE IF NOT EXISTS `device` (
    `id` INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    `mac` VARCHAR(50) UNIQUE NOT NULL COMMENT 'Dirección MAC sin dos puntos',
    `nombre` VARCHAR(100) DEFAULT NULL COMMENT 'Nombre amigable del dispositivo',
    `descripcion` TEXT COMMENT 'Descripción opcional',
    `ubicacion` VARCHAR(200) DEFAULT NULL COMMENT 'Ubicación física',
    `fecha_registro` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    `ultima_conexion` TIMESTAMP NULL DEFAULT NULL,
    `activo` TINYINT(1) DEFAULT 1 COMMENT '1=activo, 0=inactivo',
    `version_firmware` VARCHAR(20) DEFAULT NULL,
    INDEX idx_mac (mac),
    INDEX idx_activo (activo)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `measurement` (
    `id` BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    `device_id` INT UNSIGNED NOT NULL,
    `temperatura` DECIMAL(5,2) NOT NULL COMMENT 'Temperatura en °C',
    `humedad` DECIMAL(5,2) NULL COMMENT 'Humedad relativa en %',
    `presion` DECIMAL(7,2) NULL COMMENT 'Presión atmosférica en hPa',
    `calidad_aire` INT NULL COMMENT 'Calidad del aire (0-500)',
    `voltaje` DECIMAL(5,3) NULL COMMENT 'Voltaje de batería',
    `rssi` SMALLINT NULL COMMENT 'Señal WiFi en dBm',
    `fecha` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    `procesado` TINYINT(1) DEFAULT 0 COMMENT '0=sin procesar, 1=procesado',
    FOREIGN KEY (device_id) REFERENCES device(id) ON DELETE CASCADE,
    INDEX idx_device_id (device_id),
    INDEX idx_fecha (fecha),
    INDEX idx_temperatura (temperatura),
    INDEX idx_procesado (procesado)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `config` (
    `id` INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    `device_id` INT UNSIGNED NOT NULL,
    `clave` VARCHAR(50) NOT NULL COMMENT 'Ej: intervalo, temp_alerta, nombre_amigable',
    `valor` VARCHAR(500) NOT NULL,
    `tipo` ENUM('string', 'number', 'boolean', 'json') DEFAULT 'string',
    `descripcion` VARCHAR(200) DEFAULT NULL,
    `actualizable` TINYINT(1) DEFAULT 1 COMMENT '1=actualizable remotamente',
    `fecha_creacion` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    `fecha_actualizacion` TIMESTAMP NULL ON UPDATE CURRENT_TIMESTAMP,
    UNIQUE KEY `unique_device_clave` (`device_id`, `clave`),
    FOREIGN KEY (device_id) REFERENCES device(id) ON DELETE CASCADE,
    INDEX idx_device_clave (device_id, clave)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `alert` (
    `id` INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    `device_id` INT UNSIGNED NOT NULL,
    `tipo` VARCHAR(50) NOT NULL COMMENT 'temperatura_alta, temperatura_baja, desconexion',
    `valor` DECIMAL(5,2) NOT NULL,
    `limite` DECIMAL(5,2) NOT NULL,
    `mensaje` TEXT NOT NULL,
    `fecha` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    `enviada` TINYINT(1) DEFAULT 0 COMMENT '0=no enviada, 1=enviada',
    FOREIGN KEY (device_id) REFERENCES device(id) ON DELETE CASCADE,
    INDEX idx_device_fecha (device_id, fecha),
    INDEX idx_tipo_enviada (tipo, enviada)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `log` (
    `id` BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    `device_id` INT UNSIGNED NULL,
    `nivel` ENUM('DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL') DEFAULT 'INFO',
    `modulo` VARCHAR(50) DEFAULT NULL COMMENT 'local_storage, remote_storage, wifi, etc.',
    `mensaje` TEXT NOT NULL,
    `datos` JSON NULL COMMENT 'Datos adicionales en JSON',
    `fecha` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_fecha_nivel (fecha, nivel),
    INDEX idx_device_modulo (device_id, modulo),
    INDEX idx_nivel (nivel)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Configuración por defecto para nuevos dispositivos
INSERT IGNORE INTO `config` (device_id, clave, valor, tipo, descripcion) VALUES
(0, 'intervalo', '30', 'number', 'Intervalo de envío en segundos'),
(0, 'temp_alerta', '35.0', 'number', 'Temperatura máxima de alerta'),
(0, 'temp_minima', '10.0', 'number', 'Temperatura mínima de alerta'),
(0, 'humedad_maxima', '80.0', 'number', 'Humedad máxima de alerta'),
(0, 'nombre_amigable', 'ESP_{MAC}', 'string', 'Patrón para nombre automático'),
(0, 'version_api', '1.0.0', 'string', 'Versión de la API');

-- Vista: últimas mediciones por dispositivo
CREATE OR REPLACE VIEW vw_last_measurements AS
SELECT 
    d.id AS device_id,
    d.mac,
    d.nombre AS device_nombre,
    m.temperatura,
    m.humedad,
    m.fecha AS last_measurement,
    m.rssi
FROM device d
LEFT JOIN measurement m ON d.id = m.device_id
WHERE m.fecha = (SELECT MAX(fecha) FROM measurement WHERE device_id = d.id)
   OR m.id IS NULL;

-- Vista: estadísticas diarias
CREATE OR REPLACE VIEW vw_daily_stats AS
SELECT 
    device_id,
    DATE(fecha) AS dia,
    COUNT(*) AS total_mediciones,
    ROUND(AVG(temperatura), 2) AS avg_temperatura,
    ROUND(MIN(temperatura), 2) AS min_temperatura,
    ROUND(MAX(temperatura), 2) AS max_temperatura,
    ROUND(AVG(humedad), 2) AS avg_humedad
FROM measurement
GROUP BY device_id, DATE(fecha);

-- Procedimiento: limpiar datos antiguos
DELIMITER //
CREATE PROCEDURE sp_clean_old_data(IN days_to_keep INT)
BEGIN
    DECLARE cutoff_date DATE;
    SET cutoff_date = DATE_SUB(CURDATE(), INTERVAL days_to_keep DAY);
    
    -- Eliminar mediciones antiguas
    DELETE FROM measurement WHERE DATE(fecha) < cutoff_date;
    
    -- Eliminar logs antiguos (mantener errores más tiempo)
    DELETE FROM log WHERE fecha < DATE_SUB(NOW(), INTERVAL 30 DAY) AND nivel != 'ERROR';
    DELETE FROM log WHERE fecha < DATE_SUB(NOW(), INTERVAL 90 DAY);
    
    -- Eliminar alertas procesadas antiguas
    DELETE FROM alert WHERE fecha < DATE_SUB(NOW(), INTERVAL 7 DAY) AND enviada = 1;
END//
DELIMITER ;

-- Trigger: actualizar última conexión del dispositivo
DELIMITER //
CREATE TRIGGER trg_update_last_connection 
AFTER INSERT ON measurement
FOR EACH ROW
BEGIN
    UPDATE device 
    SET ultima_conexion = NEW.fecha 
    WHERE id = NEW.device_id;
END//
DELIMITER ;

-- Trigger: detectar temperatura alta
DELIMITER //
CREATE TRIGGER trg_check_temperature
AFTER INSERT ON measurement
FOR EACH ROW
BEGIN
    DECLARE temp_limit DECIMAL(5,2);
    
    -- Obtener límite de temperatura del dispositivo
    SELECT CAST(valor AS DECIMAL(5,2)) INTO temp_limit
    FROM config 
    WHERE device_id = NEW.device_id AND clave = 'temp_alerta';
    
    -- Si no tiene configuración, usar valor por defecto
    IF temp_limit IS NULL THEN
        SET temp_limit = 35.0;
    END IF;
    
    -- Insertar alerta si supera el límite
    IF NEW.temperatura > temp_limit THEN
        INSERT INTO alert (device_id, tipo, valor, limite, mensaje)
        VALUES (
            NEW.device_id,
            'temperatura_alta',
            NEW.temperatura,
            temp_limit,
            CONCAT('Temperatura alta: ', NEW.temperatura, '°C > ', temp_limit, '°C')
        );
    END IF;
END//
DELIMITER ;
