import { grommet, ThemeType } from "grommet";
import { deepMerge } from "grommet/utils";

export const theme: ThemeType = deepMerge(grommet, {
  global: {
    colors: {
      brand: "#ff692e",
    },
    font: {
      size: "18px",
      height: "20px",
    },
  },
});
