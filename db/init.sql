----------------------------------------------------------------------------
-- init.sql
-- to initialize gpies.db run the following command from GPIES/db:
--
-- sqlite3 gpies.db < clear.sql < init.sql
--
-- this will drop all tables, create them again, and insert initial data
----------------------------------------------------------------------------

----------------------------------------------------------------------------
-- TABLES
----------------------------------------------------------------------------

CREATE TABLE IF NOT EXISTS reactors
(
    id_reactor INTEGER PRIMARY KEY AUTOINCREMENT,
    creation_date DATETIME DEFAULT current_timestamp,
    species text NOT NULL DEFAULT "",
    flux float NOT NULL DEFAULT 0.0,
    temperature float NOT NULL DEFAULT 0.0,
    recombination float NOT NULL DEFAULT 0.0,
    i_bi float NOT NULL DEFAULT 0.0,
    i_tri float NOT NULL DEFAULT 0.0,
    i_quad float NOT NULL DEFAULT 0.0,
    v_bi float NOT NULL DEFAULT 0.0,
    v_tri float NOT NULL DEFAULT 0.0,
    v_quad float NOT NULL DEFAULT 0.0,
    dislocation_density_evolution float NOT NULL DEFAULT 0.0
);

CREATE TABLE IF NOT EXISTS materials
(
    id_material INTEGER PRIMARY KEY AUTOINCREMENT,
    creation_date DATETIME DEFAULT current_timestamp,
    species text NOT NULL DEFAULT "",
    i_migration float NOT NULL DEFAULT 0.0,
    v_migration float NOT NULL DEFAULT 0.0,
    i_diffusion_0 float NOT NULL DEFAULT 0.0,
    v_diffusion_0 float NOT NULL DEFAULT 0.0,
    i_formation float NOT NULL DEFAULT 0.0,
    v_formation float NOT NULL DEFAULT 0.0,
    i_binding float NOT NULL DEFAULT 0.0,
    v_binding float NOT NULL DEFAULT 0.0,
    recombination_radius float NOT NULL DEFAULT 0.0,
    i_loop_bias float NOT NULL DEFAULT 0.0,
    i_dislocation_bias float NOT NULL DEFAULT 0.0,
    i_dislocation_bias_param float NOT NULL DEFAULT 0.0,
    v_loop_bias float NOT NULL DEFAULT 0.0,
    v_dislocation_bias float NOT NULL DEFAULT 0.0,
    v_dislocation_bias_param float NOT NULL DEFAULT 0.0,
    dislocation_density_0 float NOT NULL DEFAULT 0.0,
    grain_size float NOT NULL DEFAULT 0.0,
    lattice_param float NOT NULL DEFAULT 0.0,
    burgers_vector float NOT NULL DEFAULT 0.0,
    atomic_volume float NOT NULL DEFAULT 0.0
);

CREATE TABLE IF NOT EXISTS simulations
(
    id_simulation INTEGER PRIMARY KEY AUTOINCREMENT,
    creation_date DATETIME DEFAULT current_timestamp,
    id_reactor INTEGER,
    simulation_time FLOAT NOT NULL DEFAULT 0.0,
    time_delta FLOAT NOT NULL DEFAULT 0.0,
    data TEXT
);

CREATE TABLE IF NOT EXISTS simulation_materials 
(
    id_simulation NOT NULL,
    id_material NOT NULL,
    PRIMARY KEY (id_simulation, id_material)
);


----------------------------------------------------------------------------
-- REACTORS
----------------------------------------------------------------------------

INSERT INTO reactors
(
    creation_date,
    species,
    flux,
    temperature,
    recombination,
    i_bi,
    i_tri,
    i_quad,
    v_bi,
    v_tri,
    v_quad,
    dislocation_density_evolution
)
VALUES
(
    '2024-01-01 00:00:00',
    'OSIRIS',
    2.9e-07,
    603.15,
    .3,
    .5,
    .2,
    .06,
    .06,
    .03,
    .02,
    300.
);

----------------------------------------------------------------------------
-- MATERIALS 
----------------------------------------------------------------------------

INSERT INTO materials
(
    creation_date,
    species,
    i_migration,
    v_migration,
    i_diffusion_0,
    v_diffusion_0,
    i_formation,
    v_formation,
    i_binding,
    v_binding,
    recombination_radius,
    i_loop_bias,
    i_dislocation_bias,
    i_dislocation_bias_param,
    v_loop_bias,
    v_dislocation_bias,
    v_dislocation_bias_param,
    dislocation_density_0,
    grain_size,
    lattice_param,
    burgers_vector,
    atomic_volume
)
VALUES
(
    '2024-01-01 00:00:00',
    'SA304',
    .45,
    1.35,
    1e-03,
    .6,
    4.1,
    1.7,
    .6,
    .5,
    .7e-07,
    63.,
    .8, 
    1.1,
    33.,
    .65, 
    1.,
    1e-13,
    4e-03,
    3.6e-08,
    2.5455844122715708e-08,
    1.1664e-23
);