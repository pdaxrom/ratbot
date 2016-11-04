#!/bin/sh

echo "MOTOR 0 1" > /dev/ttyACM0
echo "MOTOR 1 1" > /dev/ttyACM0
echo "LIGHT 0 1" > /dev/ttyACM0
echo "COMMIT" > /dev/ttyACM0

sleep 1

echo "MOTOR 0 -1" > /dev/ttyACM0
echo "MOTOR 1 1" > /dev/ttyACM0
echo "COMMIT" > /dev/ttyACM0

sleep 0.6

echo "MOTOR 0 1" > /dev/ttyACM0
echo "MOTOR 1 -1" > /dev/ttyACM0
echo "COMMIT" > /dev/ttyACM0

sleep 0.6

echo "MOTOR 0 -1" > /dev/ttyACM0
echo "MOTOR 1 -1" > /dev/ttyACM0
echo "LIGHT 1 1" > /dev/ttyACM0
echo "COMMIT" > /dev/ttyACM0

sleep 1

echo "MOTOR 0 0" > /dev/ttyACM0
echo "MOTOR 1 0" > /dev/ttyACM0
echo "LIGHT 0 0" > /dev/ttyACM0
echo "LIGHT 1 0" > /dev/ttyACM0
echo "COMMIT" > /dev/ttyACM0

