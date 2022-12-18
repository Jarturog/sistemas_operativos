Autores: Juan Arturo Abaurrea Calafell, Pere Antoni Prats Villalonga y Marta González Juan

- Mejoras realizadas: todas, todo lo opcional que ponía en la documentación ha sido realizado. Como por ejemplo el cd avanzado y el prompt detallado.
- Restricciones del programa: nos hemos dado cuenta de que si se ejecuta el mini shell dentro del mini shell cuando sus ejectuables tienen nombres
diferentes (por ejemplo hacer ./nivel6 y dentro hacer ./my_shell) no se tiene en cuenta a la hora de considerar si se está ejecutando el mini shell.
Esto afecta cuando se pulsa ctr+c u otras consideraciones en cuanto a los jobs.
- Problemas en la realización: durante la creación del ctr+z y al hacer pruebas con él nos hemos dado cuenta de que en algunas ocasiones imprimía el
prompt dos veces, una o ninguna.