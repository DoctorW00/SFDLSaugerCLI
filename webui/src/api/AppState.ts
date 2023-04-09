import { useState } from "react";
import { createContainer } from "react-tracked";
import { IAppState } from "./IAppState";

const useValue = () => useState<IAppState | undefined>(undefined);

export const {
  Provider,
  useTrackedState,
  useUpdate: useSetState,
} = createContainer(useValue);
