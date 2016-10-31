let groupFold keyFunc valueFunc foldFunc foldInitAcc x =
    x |> List.toSeq
      |> Seq.groupBy keyFunc
      |> Seq.map (fun (key, groupedResults) -> (key, groupedResults |> Seq.map valueFunc |> Seq.fold foldFunc foldInitAcc))
      |> Seq.toList
