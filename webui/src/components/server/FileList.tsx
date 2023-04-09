import React from "react";
import { Box, DataTable, Meter, Text } from "grommet";
import { IFile } from "../../api/IAppState";

export function FileList(props: { files: IFile[] }) {
  return (
    <DataTable
      size={"large"}
      columns={[
        {
          property: "fileName",
          header: <Text>Name</Text>,
          primary: true,
        },
        {
          property: "progress",
          header: "Progress",
          render: (data) => (
            <Box pad={{ vertical: "xsmall" }}>
              <Meter
                values={[{ value: data.progress, color: "brand" }]}
                thickness="small"
                size="small"
              />
            </Box>
          ),
        },
      ]}
      data={props.files}
    />
  );
}
