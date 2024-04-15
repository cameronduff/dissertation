import os

#run small
os.system('cd')
os.system('cd ns-allinone-3.40/ns-3.40')
os.system('./ns3 run scratch/dissertation/afdx/routing/simulations/small/afdx-network-small.cc')

#run medium
os.system('cd')
os.system('cd ns-allinone-3.40/ns-3.40')
os.system('./ns3 run scratch/dissertation/afdx/routing/simulations/medium/afdx-network-medium.cc')

#run large
os.system('cd')
os.system('cd ns-allinone-3.40/ns-3.40')
os.system('./ns3 run scratch/dissertation/afdx/routing/simulations/large/afdx-network-large.cc')
