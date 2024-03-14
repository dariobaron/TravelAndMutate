library(ape)
library(treebalance)
library(hdf5r)

metrics <- c("cherryI", "B1I", "maxWidth", "mWovermD", "maxDelW",
             "rQuartetI", "avgLeafDepI", "avgVertDep",
             "IbasedI", "maxDepth", "sackinI", "sShapeI", "totCophI",
             "totIntPathLen", "totPathLen", "varLeafDepI")

file.h5 <- H5File$new("data/treesTB.h5", mode = "a")
for (dataset_name in list.datasets(file.h5, recursive = TRUE)) {
  dataset <- file.h5[[dataset_name]]
  if (dataset$dims[[2]] == 0) {
    next
  }
  if (dataset$attr_exists("internals") && dataset$attr_exists("tips")) {
    print(paste("Processing dataset", dataset_name))
    internals <- dataset$attr_open("internals")$read()
    tips <- dataset$attr_open("tips")$read()
    branches <- dataset$read()
    tree <- list(edge = t(branches), tip.label = c(1:tips), Nnode = internals)
    class(tree) <- "phylo"
    for (metric in metrics) {
      if (!dataset$attr_exists(metric)) {
        print(paste("Computing metric", metric))
        func <- get(metric)
        value <- func(tree)
        h5attr(dataset, metric) <- value
      } else {
         print(paste("Metric", metric, "was already computed for this dataset."))
      }
    }
  }
}


file.h5$close_all()