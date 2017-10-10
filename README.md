# FreeRTOS General Example on ARM Cortex-M4
## Tools used from FreeRTOS:
 * Concurrent performance of tasks.
 * Priority utilization.
 * Queues for inter-task communications.
 * Binary sempahores for multitask synchronization.
 * Group events for more complex task synchronization and signaling (To be added)
 
## Function:
  Highest priority task creates 2 queues containing a list of color strings and RGB pins to activate associated colors.
  When a failing edge is detected on PortF pin0, a binary semaphore is released allowing synchronization with another task sending the color string via UART and writing RGB led with associated colors.
  A third task takes a peek from the queue without removing the item and used received value to set a bit in the group event byte.
