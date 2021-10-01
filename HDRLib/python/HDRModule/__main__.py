from .Managers.Estimator import EstimationManager

em = EstimationManager()

em.estimate("..\\Test\\1")
em.estimate("..\\Test\\2")
em.estimate("..\\Test\\3")
em.estimate("..\\Test\\4")
em.estimate("..\\Test\\5")

em.close()
