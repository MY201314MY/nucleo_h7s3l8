#!/bin/bash

# source ~/zephyr/.venv/bin/activate; source ~/zephyr/zephyr/zephyr-env.sh

west build -b nucleo_h7s3l8 --sysbuild -p
