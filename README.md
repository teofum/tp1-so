## Building

Para pullear y configurar el container de Docker para build y run, usar

```
make docker_pull
```

con el servicio de Docker corriendo. Este paso es de configuración y sólo es necesario hacerlo una vez.

Para buildear los componentes del proyecto, se pueden usar

```
make master
make player
make view
```

Cualquiera de los targets inicia el container de Docker.

Para buildear todo al mismo tiempo usar el target `all`, o simplemente

```
make
```

Para limpiar los ejecutables y object files,

```
make clean
```

El script `run_master.sh` ejecuta master en el container de docker.

ej: ./run_master.sh -p ./pgreedy_box ./pgreedy_box -v "./view"