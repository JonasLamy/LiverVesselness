"""
methodsParameters.py

"""

class FrangiParameters:
    def __init__(self,alpha=0.5,beta=0.5,gamma=5):
        self.alpha = alpha
        self.beta = beta
        self.gamma = gamma
        
class SatoParameters:
    def __init__(self,alpha1=0.5,alpha2=2):
        self.alpha1 = alpha1
        self.alpha2 = alpha2

class MeijeringParameters:
    def __init__(self,alpha=-0.33):
        self.alpha = alpha

class OOFParameters:
    def __init__(self,sigma=0.1):
        self.sigma = sigma

class JermanParameters:
    def __init__(self,tau=0.75):
        self.tau = tau

class ZhangParameters:
    def __init__(self,tau=0.75):
        self.tau = tau

class RORPOParameters:
    def __init__(self,core=3,dilationSize=0,verbose=True):
        self.core = core
        self.dilationSize = dilationSize
        self.verbose = verbose
