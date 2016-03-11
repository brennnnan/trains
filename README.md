# Trains

A Project for CSC360 Operating Systems in Summer 2015 that works as a train dispatcher for a bridge. The program recieves a list of a trains with the direction they are heading, their priority, and their load and cross time. It creates an array of structs housing each train and the trains all start loading at the same time.  The trains tell the controller thread when they are ready to cross (done loading), and the controller checks if they should be allowed to cross (there is a mutex that acts to guard the track from overexcited trains).  The decision about which train goes first is a combination of the load time, direction, wait time and priority of the train. When the controller thread decides which train should be allowed to cross, it notifies that train and unlocks the mutex guarding the track.

###Concepts: 
- Multithreading
- Condition Variables / Mutexes
- Signals
