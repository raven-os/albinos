# albinos_editor

## Prerequisites

Below is the list of prerequisites to compile albinos-editor on your machine:

* Nim version 0.18 minimum
* Nimble

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Build

To build the project please follow the instructions below:

```
nimble build
```

### Install

To install the project please follow the instructions below:

```
nimble install
```

### Running the tests

To run the tests for the project please follow the instructions below:

```
nimble test
```

## Architecture

The albinos configuration editor works in a complementary way with `albinos daemon service`.

The albinos editor binary comes with a dynamic library containing albinos API functionality,
This way when a client enters a command the function is directly called from the `dynamic library`.