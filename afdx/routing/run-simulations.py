import os

numOfSimulations = 100

for i in range(numOfSimulations):
    #run small
    os.system('cd')
    os.system('cd ns-allinone-3.40/ns-3.40')
    os.system(f'./ns3 run scratch/dissertation/afdx/routing/simulations/small/afdx-network-small.cc -- --flowmonName="afdx-metrics-small-{i+1}.xml"')

    #run medium
    os.system('cd')
    os.system('cd ns-allinone-3.40/ns-3.40')
    os.system(f'./ns3 run scratch/dissertation/afdx/routing/simulations/medium/afdx-network-medium.cc -- --flowmonName="afdx-metrics-medium-{i+1}.xml"')

    #run large
    os.system('cd')
    os.system('cd ns-allinone-3.40/ns-3.40')
    os.system(f'./ns3 run scratch/dissertation/afdx/routing/simulations/large/afdx-network-large.cc -- --flowmonName="afdx-metrics-large-{i+1}.xml"')
