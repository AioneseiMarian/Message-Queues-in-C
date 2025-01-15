# Message Queues 

## We made a system with Message Queueing based on topics, subtopics and a filter 

   A Message Queue system in which a subscriber can start a communication channel based on a topic and a subtopic (strings) and subscribers can subscribe to those channels and receive the messages.

   The Server centralizes the messages in a structure formed from a Hashtable for searching topics, each Entry is a RB Tree that organizes based on the subtopic and the Tree stores Queues of Messages to ensure that new subscribers also get messages from the past. 
   
   A thread listens to events with epoll_wait and stores new subscribers or new messages into a Queue from where we have a Thread Pool. Each thread from the pool gets events, ensuring atomicity using semaphores and mutexes. 
   We also have a mechanism in which the Server saves the messages into a json file and loads them at startup. 

   The Subscribers have the opportunity to subscribe based on topic, subtopic and also a filter(optional), named interest(they will only get messages based on that filter). Then they store them into a json file. 

   To Do: We will develop a library with wrapper functions so you can make your own Subscriber/Publisher app. 
  


